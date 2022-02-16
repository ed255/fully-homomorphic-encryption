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

#include <stdio.h>
#include <time.h>

#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "palisade/binfhe/binfhecontext.h"
#include "transpiler/data/palisade_data.h"
#include "transpiler/examples/string_cap/string_cap.h"
#include "xls/common/logging/logging.h"

#ifdef USE_INTERPRETED_PALISADE
#include "transpiler/examples/string_cap/string_cap_interpreted_palisade.h"
#elif defined(USE_YOSYS_INTERPRETED_PALISADE)
#include "transpiler/examples/string_cap/string_cap_yosys_interpreted_palisade.h"
#else
#include "transpiler/examples/string_cap/string_cap_palisade.h"
#endif

constexpr int kMainMinimumLambda = 120;

void PalisadeStringCap(PalisadeString& ciphertext, lbcrypto::BinFHEContext cc) {
  absl::Time start_time = absl::Now();
  double cpu_start_time = clock();
  std::cout << "Starting!" << std::endl;
  XLS_CHECK_OK(CapitalizeString(ciphertext, cc));
  std::cout << "\t\t\t\t\tTotal time: "
            << absl::ToDoubleSeconds(absl::Now() - start_time) << " secs"
            << std::endl;
  std::cout << "\t\t\t\t\t  CPU time: "
            << (clock() - cpu_start_time) / 1'000'000 << " secs" << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: string_cap_palisade_testbench string_input\n\n");
    return 1;
  }

  std::string input = argv[1];
  input.resize(MAX_LENGTH, '\0');

  std::cout << "Starting key generation..." << std::endl;
  const absl::Time key_gen_start = absl::Now();

  auto cc = lbcrypto::BinFHEContext();
  cc.GenerateBinFHEContext(lbcrypto::MEDIUM);

  std::cout << "Generating the secret key..." << std::endl;

  // Generate the secret key
  auto sk = cc.KeyGen();

  std::cout << "Generating the bootstrapping keys..." << std::endl;

  // Generate the bootstrapping keys (refresh and switching keys)
  cc.BTKeyGen(sk);

  std::cout << "Completed key generation ("
            << absl::ToDoubleSeconds(absl::Now() - key_gen_start) << " secs)"
            << std::endl;

  std::string plaintext(input);
  std::cout << "plaintext: '" << plaintext << "'" << std::endl;

  std::cout << "Encrypting inputs..." << std::endl;
  const absl::Time encryption_start = absl::Now();

  // Encrypt data
  auto ciphertext = PalisadeString::Encrypt(plaintext, cc, sk);

  std::cout << "Encryption done (" << ciphertext.bit_width() << " bits, "
            << absl::ToDoubleSeconds(absl::Now() - encryption_start) << " secs)"
            << std::endl;

  std::cout << "Initial state check by decryption: " << std::endl;
  const absl::Time initial_decryption_start = absl::Now();
  std::cout << ciphertext.Decrypt(sk) << std::endl;
  std::cout << "(took "
            << absl::ToDoubleSeconds(absl::Now() - initial_decryption_start)
            << " secs)" << std::endl;

  std::cout << "\t\t\t\t\tServer side computation:" << std::endl;
  // Perform string capitalization
  PalisadeStringCap(ciphertext, cc);
  std::cout << "\t\t\t\t\tComputation done" << std::endl;

  const absl::Time decryption_start = absl::Now();
  std::cout << "Decrypted result: " << ciphertext.Decrypt(sk) << "\n";
  std::cout << "Decryption done ("
            << absl::ToDoubleSeconds(absl::Now() - decryption_start) << " secs)"
            << std::endl;
}
