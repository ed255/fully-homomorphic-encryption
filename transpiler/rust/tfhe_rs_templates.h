#ifndef FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_RUST_TFHE_RS_TEMPLATES_H_
#define FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_RUST_TFHE_RS_TEMPLATES_H_

#include "absl/strings/string_view.h"

namespace fhe {
namespace rust {
namespace transpiler {

// Constants used for string templating, these are placed here so the tests
// can reuse them.
constexpr absl::string_view kCodegenTemplate = R"rust(
use rayon::prelude::*;
use std::collections::HashMap;

use phantom_zone::*;

type Ciphertext = FheBool;

enum GateInput {
    Arg(usize, usize), // arg + index
    Output(usize), // reuse of output wire
    Tv(usize),  // temp value
    Cst(bool),  // constant
}

use GateInput::*;

#[derive(PartialEq, Eq, Hash)]
enum CellType {
    AND2,
    NAND2,
    XOR2,
    XNOR2,
    OR2,
    NOR2,
    INV,
    // TODO: Add back MUX2
}

use CellType::*;

$gate_levels

fn prune(temp_nodes: &mut HashMap<usize, Ciphertext>, temp_node_ids: &[usize]) {
  for x in temp_node_ids {
    temp_nodes.remove(&x);
  }
}

pub fn $function_signature {
    let parameter_set = get_active_parameter_set();
    rayon::ThreadPoolBuilder::new()
        .build_scoped(
            |thread| {
                set_parameter_set(parameter_set);
                thread.run()
            },
            |pool| pool.install(|| {

                let args: &[&Vec<Ciphertext>] = &[$ordered_params];

                let mut temp_nodes = HashMap::new();
                let mut $output_stem = Vec::new();
                $output_stem.resize($num_outputs, None);

                let mut run_level = |
                temp_nodes: &mut HashMap<usize, Ciphertext>,
                tasks: &[((usize, bool, CellType), &[GateInput])]
                | {
                    let updates = tasks
                        .into_par_iter()
                        .map(|(k, task_args)| {
                            let (id, is_output, celltype) = k;
                            let task_args = task_args.into_iter()
                            .map(|arg| match arg {
                                Cst(false) => todo!(),
                                Cst(true) => todo!(),
                                Arg(pos, ndx) => &args[*pos][*ndx],
                                Tv(ndx) => &temp_nodes[ndx],
                                Output(ndx) => &$output_stem[*ndx]
                                            .as_ref()
                                            .expect(&format!("Output node {ndx} not found")),
                            }).collect::<Vec<_>>();

                            let gate_func = |args: &[&Ciphertext]| match celltype {
                                AND2 => args[0] & args[1],
                                NAND2 => args[0].nand(args[1]),
                                OR2 => args[0] | args[1],
                                NOR2 => args[0].nor(args[1]),
                                XOR2 => args[0] ^ args[1],
                                XNOR2 => args[0].xnor(args[1]),
                                INV => !args[0],
                            };
                            
                            ((*id, *is_output), gate_func(&task_args))
                        })
                        .collect::<Vec<_>>();
                    updates.into_iter().for_each(|(k, v)| {
                        let (index, is_output) = k;
                        if is_output {
                            $output_stem[index] = Some(v);
                        } else {
                            temp_nodes.insert(index, v);
                        }
                    });
                };

            $run_level_ops

            $output_assignment_block

                $return_statement
            }),
        )
        .unwrap()
}

)rust";

// The data of a level's gate ops must be defined statically, or else the
// rustc (LLVM) optimizing passes will take forever.
//
// e.g.,
//
//   const LEVEL_3: [((usize, bool, CellType), &[GateInput]); 8] = [
//       ((1, true, LUT3(6)), &[Arg(0, 0), Arg(0, 1), Cst(false)]),
//       ((2, true, LUT3(120)), &[Arg(0, 0), Arg(0, 1), Arg(0, 2)]),
//       ...
//   ];
//
constexpr absl::string_view kLevelTemplate = R"rust(
static LEVEL_%d: [((usize, bool, CellType), &[GateInput]); %d] = [
%s
];)rust";

// The data specifying, for each LEVEL_K, the set of gate output ids
// that can be safely deleted from memory after LEVEL_K is run.
constexpr absl::string_view kPruneTemplate = R"rust(
static PRUNE_%d: [usize; %d] = [
%s
];)rust";

// A single gate operation defined as constant data
//
//   ((output_id, is_output, LUT3(lut_defn)), &[arglist]),
//
// e.g.,
//
//   ((2, false, LUT3(120)), &[Arg(0, 0), Arg(0, 1), Arg(0, 2)]),
// or
//   ((2, false, AND), &[Arg(0, 0), Arg(0, 1), Arg(0, 2)]),
//
constexpr absl::string_view kTaskTemplate = "    ((%d, %s, %s), &[%s]),";

// The commands used to run the levels defined by kLevelTemplate above.
//
// e.g.,
//
//  run_level(&LEVEL_0);
//  run_level(&LEVEL_1);
//  ...
//
constexpr absl::string_view kRunLevelTemplate =
    "    run_level(&mut temp_nodes, &LEVEL_%d);";

// The commands used to clean up old temp_nodes values that are no longer
// needed. Necessary for large programs with 1M or more gates to conserve RAM.
//
// e.g.,
//
//  run_level(&LEVEL_0);
//  prune(&PRUNE_0);
//  run_level(&LEVEL_1);
//  prune(&PRUNE_1);
//  ...
//
constexpr absl::string_view kRunPruneTemplate =
    "    prune(&mut temp_nodes, &PRUNE_%d);";

// This is needed for the output section of a program,
// when the output values from the yosys netlist must be split off into multiple
// output values for the caller of the code-genned function.
//
// e.g., the following writes the first 16 bits of `out` to an outparameter,
// then returns the remaining bits.
//
//     outputValue[0] = out[0].clone();
//     outputValue[1] = out[1].clone();
//     outputValue[2] = out[2].clone();
//     outputValue[3] = out[3].clone();
//     outputValue[4] = out[4].clone();
//     outputValue[5] = out[5].clone();
//     outputValue[6] = out[6].clone();
//     outputValue[7] = out[7].clone();
//     outputValue[8] = out[8].clone();
//     outputValue[9] = out[9].clone();
//     outputValue[10] = out[10].clone();
//     outputValue[11] = out[11].clone();
//     outputValue[12] = out[12].clone();
//     outputValue[13] = out[13].clone();
//     outputValue[14] = out[14].clone();
//     outputValue[15] = out[15].clone();
//     out = out.split_off(16);
//
constexpr absl::string_view kAssignmentTemplate = "    %s[%d] = %s.clone();";
constexpr absl::string_view kSplitTemplate = "    %s = %s.split_off(%d);";

}  // namespace transpiler
}  // namespace rust
}  // namespace fhe

#endif  // FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_RUST_TFHE_RS_TEMPLATES_H_
