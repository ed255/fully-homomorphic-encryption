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

#include "collision_test.h"

#pragma hls_top
bool entrypoint(unsigned char x, unsigned char y) {
  // const int boundaries[] = {0, 0, 1, 01};
  // const int len = 2;

/*
// Case 1
const int boundaries[] = {
7, 1, 17, 15, 14, 8, 6, 5, 27, 2, 1, 5, 13, 14, 1, 12
, 26, 14, 28, 17, 0, 10, 27, 21, 17, 9, 13, 21, 6, 5, 24, 6
, 22, 22, 16, 2, 29, 7, 24, 5, 18, 23, 12, 4, 2, 14, 18, 5
, 14, 6, 24, 17, 29, 23, 10, 23, 22, 13, 17, 4, 10, 15, 10, 29
, 24, 17, 14, 20, 3, 14, 2, 20, 25, 17, 4, 13, 20, 13, 31, 25
, 29, 9, 16, 8, 15, 16, 27, 25, 23, 14, 8, 31, 5, 3, 7, 9
, 10, 27, 4, 24, 24, 29, 16, 0, 7, 17, 21, 7, 18, 27, 10, 29
, 0, 16, 11, 6, 19, 12, 9, 23, 10, 0, 20, 31, 1, 7, 23, 19
, 15, 3, 15, 5, 5, 31, 4, 8, 8, 30, 10, 16, 27, 13, 12, 19
, 25, 23, 28, 28, 7, 15, 14, 4, 21, 1, 14, 14, 0, 4, 3, 14
, 4, 2, 21, 4, 15, 17, 31, 13, 8, 30, 15, 30, 26, 12, 6, 6
, 27, 22, 27, 26, 29, 3, 6, 3, 25, 21, 6, 15, 12, 12, 28, 8
, 27, 11, 17, 29, 15, 4, 28, 6, 3, 0, 5, 15, 10, 26, 31, 30
, 13, 25, 3, 10, 24, 0, 24, 16, 29, 18, 27, 31, 9, 12, 18, 13
, 3, 3, 20, 3, 3, 30, 10, 3, 5, 11, 4, 4, 15, 25, 7, 15
, 2, 5, 26, 20, 16, 13, 20, 15, 16, 25, 8, 19, 29, 20, 4, 0
};
const int len = 128;
*/

// Case 2
const int boundaries[] = {
0, 0, 31, 0, 0, 0, 0, 31, 0, 1, 31, 1, 1, 0, 1, 31
, 0, 2, 31, 2, 2, 0, 2, 31, 0, 3, 31, 3, 3, 0, 3, 31
, 0, 4, 31, 4, 4, 0, 4, 31, 0, 5, 31, 5, 5, 0, 5, 31
, 0, 6, 31, 6, 6, 0, 6, 31, 0, 7, 31, 7, 7, 0, 7, 31
, 0, 8, 31, 8, 8, 0, 8, 31, 0, 9, 31, 9, 9, 0, 9, 31
, 0, 10, 31, 10, 10, 0, 10, 31, 0, 11, 31, 11, 11, 0, 11, 31
, 0, 12, 31, 12, 12, 0, 12, 31, 0, 13, 31, 13, 13, 0, 13, 31
, 0, 14, 31, 14, 14, 0, 14, 31, 0, 15, 31, 15, 15, 0, 15, 31
, 0, 16, 31, 16, 16, 0, 16, 31, 0, 17, 31, 17, 17, 0, 17, 31
, 0, 18, 31, 18, 18, 0, 18, 31, 0, 19, 31, 19, 19, 0, 19, 31
, 0, 20, 31, 20, 20, 0, 20, 31, 0, 21, 31, 21, 21, 0, 21, 31
, 0, 22, 31, 22, 22, 0, 22, 31, 0, 23, 31, 23, 23, 0, 23, 31
, 0, 24, 31, 24, 24, 0, 24, 31, 0, 25, 31, 25, 25, 0, 25, 31
, 0, 26, 31, 26, 26, 0, 26, 31, 0, 27, 31, 27, 27, 0, 27, 31
, 0, 28, 31, 28, 28, 0, 28, 31, 0, 29, 31, 29, 29, 0, 29, 31
, 0, 30, 31, 30, 30, 0, 30, 31, 0, 31, 31, 31, 31, 0, 31, 31
};
const int len = 128;

  bool ok = true;
  #pragma hls_unroll yes
  for (int i = 0; i < len; i++) {
    bool collision = (x == boundaries[i*2]) && (y == boundaries[i*2+1]);
    ok &= !collision;
  }
  return ok;
}
