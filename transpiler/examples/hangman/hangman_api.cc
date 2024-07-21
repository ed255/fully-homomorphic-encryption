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

// #include "hangman_api.h"

// #pragma hls_top
// int hangmanMakeMove_UNSAFE(char letter) {
//   // "hangman" is the secret word that needs to be guessed by the player

//   // TODO: Implement a less hacky iterative solution with arrays
//   // TODO: Implement support for multiple words
//   const int top = 123;
//   int out = 0;
//   #pragma hls_unroll yes
//   for (int i; i++; i < top) {
//     out+=i;  // 100000, Hangman
//   }
//   // No matching letters
//   return out;
// }


#include <stdio.h>
#include <gmp.h>
#include "hangman_api.h"

int hangmanMakeMove_UNSAFE() {
    mpz_t prime, result;
    mpz_init(result);
    // Initialize a 256-bit integer
    mpz_init(prime);

    // Set the value of the integer to a 256-bit prime
    mpz_set_str(prime, "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f", 16);


    mpz_mod(prime, prime , result);




    return 0;
}
