// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "transpiler/palisade_runner.h"

#include <pthread.h>

#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/text_format.h"
#include "palisade/binfhe/binfhecontext.h"
#include "transpiler/abstract_xls_transpiler.h"
#include "xls/common/file/filesystem.h"
#include "xls/common/status/status_macros.h"
#include "xls/contrib/xlscc/metadata_output.pb.h"
#include "xls/ir/function.h"
#include "xls/ir/ir_parser.h"
#include "xls/ir/node.h"
#include "xls/ir/node_iterator.h"
#include "xls/ir/nodes.h"
#include "xls/ir/package.h"
#include "xls/ir/type.h"

namespace fully_homomorphic_encryption {
namespace transpiler {

PalisadeRunner::PalisadeRunner(std::unique_ptr<xls::Package> package,
                               xlscc_metadata::MetadataOutput metadata)
    : package_(std::move(package)), metadata_(metadata) {
  threads_should_exit_.store(false);

  XLS_CHECK(0 == pthread_mutex_init(&lock_, nullptr));
  XLS_CHECK(0 == sem_init(&input_sem_, 1, 0));
  XLS_CHECK(0 == sem_init(&output_sem_, 1, 0));

  // *2 for hyperthreading opportunities
  const int numCPU = sysconf(_SC_NPROCESSORS_ONLN) * 2;
  for (int c = 0; c < numCPU; ++c) {
    pthread_t new_thread;
    XLS_CHECK(0 == pthread_create(&new_thread, nullptr,
                                  PalisadeRunner::ThreadBodyStatic,
                                  (void*)this));
    threads_.push_back(new_thread);
  }
}

PalisadeRunner::~PalisadeRunner() {
  threads_should_exit_.store(true);

  // Wake up threads
  for (pthread_t pt : threads_) {
    (void)pt;
    sem_post(&input_sem_);
  }
  // Wait for exit
  for (pthread_t pt : threads_) {
    pthread_join(pt, nullptr);
  }

  XLS_CHECK(0 == pthread_mutex_destroy(&lock_));
  XLS_CHECK(0 == sem_destroy(&input_sem_));
  XLS_CHECK(0 == sem_destroy(&output_sem_));
}

absl::Status PalisadeRunner::HandleBitSlice(
    lbcrypto::LWECiphertext& result, const xls::BitSlice* bit_slice,
    absl::flat_hash_map<std::string, absl::Span<lbcrypto::LWECiphertext>> args,
    lbcrypto::BinFHEContext cc) {
  xls::Node* operand = bit_slice->operand(0);

  int slice_idx = bit_slice->start();

  for (; !operand->Is<xls::Param>(); operand = operand->operand(0)) {
    if (operand->Is<xls::TupleIndex>()) {
      int slice_idx_adj = 0;
      xls::TupleIndex* tuple_index = operand->As<xls::TupleIndex>();
      int64_t actual_tuple_index = tuple_index->index();
      xls::TupleType* tuple =
          tuple_index->operand(0)->GetType()->AsTupleOrDie();
      for (int i = 0; i < actual_tuple_index; i++) {
        slice_idx_adj += tuple->element_type(i)->GetFlatBitCount();
      }
      slice_idx += slice_idx_adj;
    } else if (operand->Is<xls::ArrayIndex>()) {
      // If we're slicing into an array index, then we just need to get
      // the bit offset of index in the bit vector that represents the array
      // (in booleanified space).
      const xls::ArrayIndex* array_index = operand->As<xls::ArrayIndex>();
      XLS_ASSIGN_OR_RETURN(const xls::ArrayType* array_type,
                           array_index->array()->GetType()->AsArray());

      // TODO: Only literal indices into single-dimensional arrays
      // are currently supported. To extend past 1-d, we'll need to walk up the
      // array index chain, determining at each step the offset from element 0,
      // and pass that back down here.
      absl::Span<xls::Node* const> indices = array_index->indices();
      // TODO: Only single-dimensional indexes are supported at the
      // moment.
      if (indices.size() != 1) {
        return absl::InvalidArgumentError(
            "Only single-dimensional arrays/array indices are supported.");
      }
      if (!indices[0]->Is<xls::Literal>()) {
        return absl::InvalidArgumentError(
            "Only literal indexes into arrays are supported.");
      }
      xls::Literal* literal = indices[0]->As<xls::Literal>();

      XLS_ASSIGN_OR_RETURN(int64_t concrete_index,
                           literal->value().bits().ToUint64());
      slice_idx +=
          array_type->element_type()->GetFlatBitCount() * concrete_index;
    }

    // Verify that the only things allowed in a BitSlice chain are array
    // indexes, tuple indexes, other bit slices, and the eventual params.
    XLS_CHECK(operand->Is<xls::ArrayIndex>() || operand->Is<xls::BitSlice>() ||
              operand->Is<xls::Param>() || operand->Is<xls::TupleIndex>())
        << "Invalid BitSlice operand: " << operand->ToString();
  }

  std::string param_name = operand->GetName();
  auto found_arg = args.find(param_name);
  XLS_CHECK(found_arg != args.end());
  result = found_arg->second[slice_idx];
  return absl::OkStatus();
}

absl::Status PalisadeRunner::CollectNodeValue(
    const xls::Node* node, absl::Span<lbcrypto::LWECiphertext> output_arg,
    int output_offset,
    const absl::flat_hash_map</*id=*/uint64_t, lbcrypto::LWECiphertext>& values,
    lbcrypto::BinFHEContext cc) {
  xls::Type* type = node->GetType();
  std::string outputs;
  switch (type->kind()) {
    case xls::TypeKind::kBits: {
      // If this is a single bit, then we can [finally] emit the copy.
      int64_t bit_count = type->GetFlatBitCount();
      if (bit_count == 1) {
        // We can't handle concats in the transpiler, so if our single-bit is
        // one, walk up a level.
        while (node->Is<xls::Concat>()) {
          node = node->operand(0);
        }
        // Copy this node to the appropriate bit of the output.
        output_arg[output_offset] = values.at(node->id());
        break;
      }

      // Otherwise, keep drilling down. Note that we iterate over bits in
      // "reverse" order, to match XLS' internal big-endian bit ordering (NOT
      // BYTE ORDERING) to the currently assumed little-endian bit ordering of
      // the host.
      for (int i = 0; i < bit_count; i++) {
        XLS_RETURN_IF_ERROR(
            CollectNodeValue(node->operand(i), output_arg,
                             output_offset + (bit_count - i - 1), values, cc));
      }
      break;
    }
    case xls::TypeKind::kArray: {
      const xls::ArrayType* array_type = type->AsArrayOrDie();
      int64_t stride = array_type->element_type()->GetFlatBitCount();
      for (int i = 0; i < array_type->size(); i++) {
        XLS_RETURN_IF_ERROR(CollectNodeValue(node->operand(i), output_arg,
                                             output_offset + i * stride, values,
                                             cc));
      }
      break;
    }
    case xls::TypeKind::kTuple: {
      // TODO: Populating output tuple types can be dangerous -
      // if they correspond to C structures, then there could be strange
      // issues such as host-native structure layout not matching the packed
      // layout used inside XLS, e.g.,
      // struct Foo {
      //  char a;
      //  short b;
      //  int c;
      // };
      // may have padding inserted around some elements. User beware (for now,
      // at least).
      const xls::TupleType* tuple_type = type->AsTupleOrDie();
      int64_t sub_offset = 0;
      for (int i = 0; i < tuple_type->size(); i++) {
        XLS_RETURN_IF_ERROR(CollectNodeValue(node->operand(i), output_arg,
                                             output_offset + sub_offset, values,
                                             cc));
        sub_offset += node->operand(i)->GetType()->GetFlatBitCount();
      }
      break;
    }
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("Unsupported type kind: ", type->kind()));
  }
  return absl::OkStatus();
}

absl::Status PalisadeRunner::CollectOutputs(
    absl::Span<lbcrypto::LWECiphertext> result,
    absl::flat_hash_map<std::string, absl::Span<lbcrypto::LWECiphertext>> args,
    const absl::flat_hash_map</*id=*/uint64_t, lbcrypto::LWECiphertext>& values,
    lbcrypto::BinFHEContext cc) {
  XLS_ASSIGN_OR_RETURN(auto function, GetEntry());
  const xls::Node* return_value = function->return_value();

  std::vector<const xls::Node*> elements;
  const xls::Type* type = return_value->GetType();

  // The return value can be a tuple in two cases:
  //
  // Case A:
  //    StructA foo(StructB in)
  //    void foo(Struct &in)
  // Case B:
  //    StructR foo(StructA a, StructB &b)
  //    StructR foo(StructA &a, StructB b)
  //    void foo(StructA &a, StructB &b)
  //
  // IOW, Case A is when a function returns a single struct (either explicitly
  // or as an non-const reference, this struct is represented as a tuple in the
  // return type.  CaseB is when multiple values are returned (via some
  // combination of a return value and one or more non-const references, or a
  // void return and two or more non-const references.
  //
  // Thus in case A we want to append to elements, while in case B we want to
  // splice the results tuple in.
  //
  // In both cases, type->kind() will be a kTuple, so we need an additional
  // check to tell cases A and B apart.

  auto top_func_proto = metadata_.top_func_proto();
  auto params = top_func_proto.params();
  auto return_type = top_func_proto.return_type();

  int num_out_params = xlscc_metadata::GetNumOutParams(metadata_);

  if (type->kind() == xls::TypeKind::kTuple) {
    if (num_out_params != 1) {
      elements.insert(elements.begin(), return_value->operands().begin(),
                      return_value->operands().end());
    } else {
      elements.push_back(return_value);
    }
  } else {
    elements.push_back(return_value);
  }

  if (elements.empty()) {
    return absl::OkStatus();
  }

  int output_idx = 0;
  if (metadata_.top_func_proto().return_type().has_as_void()) {
    if (!result.empty()) {
      return absl::FailedPreconditionError(
          "return value requested for a void-returning function");
    }
  } else {
    if (result.empty()) {
      return absl::FailedPreconditionError(
          "missing return value for a value-returning function");
    }
    XLS_RETURN_IF_ERROR(
        CollectNodeValue(elements[output_idx++], result, 0, values, cc));
  }

  const auto& fn_params = metadata_.top_func_proto().params();
  int param_idx = 0;
  for (; output_idx < elements.size(); output_idx++) {
    const xlscc_metadata::FunctionParameter* param;
    while (true) {
      if (param_idx == fn_params.size()) {
        return absl::InternalError(absl::StrCat(
            "No matching in/out param for output %d: ", output_idx));
      }

      param = &fn_params[param_idx++];
      if (!param->is_const() && param->is_reference()) {
        break;
      }

      if (!param->has_type()) {
        return absl::InternalError(
            absl::StrCat("Parameter %s has no type.", param->name()));
      }
    }

    XLS_RETURN_IF_ERROR(CollectNodeValue(elements[output_idx],
                                         args[param->name()], 0, values, cc));
  }

  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<PalisadeRunner>> PalisadeRunner::CreateFromFile(
    absl::string_view ir_path, absl::string_view metadata_path) {
  XLS_ASSIGN_OR_RETURN(std::string ir_text, xls::GetFileContents(ir_path));
  XLS_ASSIGN_OR_RETURN(auto package, xls::Parser::ParsePackage(ir_text));

  XLS_ASSIGN_OR_RETURN(std::string metadata_binary,
                       xls::GetFileContents(metadata_path));
  xlscc_metadata::MetadataOutput metadata;
  if (!metadata.ParseFromString(metadata_binary)) {
    return absl::InvalidArgumentError(
        "Could not parse function metadata proto.");
  }
  return std::make_unique<PalisadeRunner>(std::move(package),
                                          std::move(metadata));
}

absl::StatusOr<std::unique_ptr<PalisadeRunner>>
PalisadeRunner::CreateFromStrings(absl::string_view xls_package,
                                  absl::string_view metadata_text) {
  XLS_ASSIGN_OR_RETURN(auto package, xls::Parser::ParsePackage(xls_package));

  xlscc_metadata::MetadataOutput metadata;
  if (!google::protobuf::TextFormat::ParseFromString(std::string(metadata_text),
                                                     &metadata)) {
    return absl::InvalidArgumentError(
        "Could not parse function metadata proto.");
  }

  return std::make_unique<PalisadeRunner>(std::move(package),
                                          std::move(metadata));
}

absl::StatusOr<lbcrypto::LWECiphertext> PalisadeRunner::EvalSingleOp(
    xls::Node* n, std::vector<lbcrypto::LWECiphertext> operands,
    const absl::flat_hash_map<std::string, absl::Span<lbcrypto::LWECiphertext>>
        args,
    lbcrypto::BinFHEContext cc) {
  XLS_CHECK(n != nullptr);
  auto node_type = n->op();
  if (node_type == xls::Op::kArray || node_type == xls::Op::kArrayIndex ||
      node_type == xls::Op::kConcat || node_type == xls::Op::kParam ||
      node_type == xls::Op::kShrl || node_type == xls::Op::kTuple ||
      node_type == xls::Op::kTupleIndex) {
    // These are all handled as operands to slice nodes.
    return nullptr;
  }

  lbcrypto::LWECiphertext out;
  switch (node_type) {
    case xls::Op::kBitSlice: {
      // Slices should be of parameters with width 1.
      auto slice = n->As<xls::BitSlice>();
      XLS_RETURN_IF_ERROR(HandleBitSlice(out, slice, args, cc));
    } break;
    case xls::Op::kLiteral: {
      // Literals must be bits with width 1, or else used purely as array
      // indices.
      auto literal = n->As<xls::Literal>();
      auto bits = literal->GetType()->AsBitsOrDie();
      if (bits->bit_count() == 1) {
        out = cc.EvalConstant(literal->value().IsAllZeros() ? 0 : 1);
      } else {
        // We allow literals strictly for pulling values out of [param]
        // arrays.
        for (const xls::Node* user : literal->users()) {
          if (!user->Is<xls::ArrayIndex>()) {
            XLS_LOG(FATAL) << "Unsupported literal: " << n->ToString();
          }
        }
        return nullptr;
      }
    } break;
    case xls::Op::kAnd: {
      XLS_CHECK(operands.size() == 2);
      XLS_CHECK(operands[0] != nullptr);
      XLS_CHECK(operands[1] != nullptr);
      if (operands[0] == operands[1]) {
        out = operands[0];
      } else {
        out = cc.EvalBinGate(lbcrypto::AND, operands[0], operands[1]);
      }
    } break;
    case xls::Op::kOr: {
      XLS_CHECK(operands.size() == 2);
      XLS_CHECK(operands[0] != nullptr);
      XLS_CHECK(operands[1] != nullptr);
      if (operands[0] == operands[1]) {
        out = operands[0];
      } else {
        out = cc.EvalBinGate(lbcrypto::OR, operands[0], operands[1]);
      }
    } break;
    case xls::Op::kNot: {
      XLS_CHECK(operands.size() == 1);
      XLS_CHECK(operands[0] != nullptr);
      out = cc.EvalNOT(operands[0]);
    } break;
    default:
      XLS_LOG(FATAL) << "Unsupported node: " << n->ToString();
      return nullptr;
  }
  return out;
}

absl::Status PalisadeRunner::Run(
    absl::Span<lbcrypto::LWECiphertext> result,
    absl::flat_hash_map<std::string, absl::Span<lbcrypto::LWECiphertext>> args,
    lbcrypto::BinFHEContext cc) {
  XLS_CHECK(input_queue_.empty());
  XLS_CHECK(output_queue_.empty());

  const_args_ = args;
  const_cc_ = cc;

  XLS_ASSIGN_OR_RETURN(auto entry, GetEntry());
  auto type = entry->GetType();

  // Arguments must match and all types must be bits.
  XLS_CHECK(type->parameter_count() == args.size());
  for (auto n : entry->params()) {
    XLS_CHECK(n != nullptr);
    XLS_CHECK(args.contains(n->name()));
  }

  auto return_value = entry->return_value();
  XLS_CHECK(return_value != nullptr);

  // Map of intermediate LWECiphertext, indexed by node id.
  absl::flat_hash_map<uint64_t, lbcrypto::LWECiphertext> values;

  absl::flat_hash_set<xls::Node*> unevaluated;
  unevaluated.insert(entry->nodes().begin(), entry->nodes().end());

  while (!unevaluated.empty()) {
    // Threads should not be running right now
    XLS_CHECK(input_queue_.empty());
    XLS_CHECK(output_queue_.empty());

    // Scan ahead and find nodes that are ready to be evaluated
    for (xls::Node* n : unevaluated) {
      std::vector<lbcrypto::LWECiphertext> operands;
      operands.resize(n->operand_count());

      bool all_operands_ready = true;

      for (int opi = 0; opi < n->operand_count(); ++opi) {
        xls::Node* opn = n->operand(opi);
        auto found_val = values.find(opn->id());
        if (found_val == values.end()) {
          all_operands_ready = false;
          break;
        }
        operands[opi] = found_val->second;
      }

      if (all_operands_ready) {
        input_queue_.push(NodeToEval(n, operands));
      }
    }

    const int n_to_run = input_queue_.size();

    // Unblock the worker threads
    for (int i = 0; i < n_to_run; ++i) {
      sem_post(&input_sem_);
    }

    // Wait for output from the worker threads
    for (int i = 0; i < n_to_run; ++i) {
      sem_wait(&output_sem_);
    }

    // Process output
    while (!output_queue_.empty()) {
      NodeFromEval from_eval = output_queue_.front();
      output_queue_.pop();

      xls::Node* n = std::get<0>(from_eval);

      // Even if the result was nullptr, mark the op as complete
      XLS_CHECK(!values.contains(n->id()));
      values[n->id()] = std::get<1>(from_eval);

      unevaluated.erase(n);
    }
  }

  // Copy the return value.
  XLS_RETURN_IF_ERROR(CollectOutputs(result, args, values, cc));

  return absl::OkStatus();
}

void* PalisadeRunner::ThreadBodyStatic(void* runner) {
  XLS_CHECK(reinterpret_cast<PalisadeRunner*>(runner)->ThreadBody().ok());
  return 0;
}

absl::Status PalisadeRunner::ThreadBody() {
  while (true) {
    // Wait for the signal from the main thread
    sem_wait(&input_sem_);

    // Check if the signal is to exit
    if (threads_should_exit_.load()) {
      return absl::OkStatus();
    }

    // Get an input safely
    pthread_mutex_lock(&lock_);
    NodeToEval to_eval = input_queue_.front();
    input_queue_.pop();
    pthread_mutex_unlock(&lock_);

    // Process the input
    xls::Node* n = std::get<0>(to_eval);
    lbcrypto::LWECiphertext out = nullptr;
    XLS_ASSIGN_OR_RETURN(
        out, EvalSingleOp(n, std::get<1>(to_eval), const_args_, const_cc_));

    // Save the output safely
    pthread_mutex_lock(&lock_);
    output_queue_.push(NodeFromEval(n, out));
    pthread_mutex_unlock(&lock_);

    // Signal the main thread
    sem_post(&output_sem_);
  }
}

}  // namespace transpiler
}  // namespace fully_homomorphic_encryption
