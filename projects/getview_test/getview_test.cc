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

#include "getview_test.h"

#pragma hls_top
View entrypoint(State state, unsigned char x, unsigned char y) {
  struct View view;

  #pragma hls_unroll yes
  for (int i=0; i<5; i++) {
    unsigned char qx = x + i - 2;
    unsigned char cell_idx_partial = i*5;
    #pragma hls_unroll yes
    for (int j=0; j<5; j++) {
      unsigned char qy = y + j - 2;
      unsigned char cell_idx = cell_idx_partial + j;
      struct CellData data;
      #pragma hls_unroll yes
      for (int k=0; k<NUM_OBJS; k++) {
        struct Object obj = state.objects[k];
        if (qx == obj.x && qy == obj.y) {
           data = obj.data;
        }
      }
      view.cells[cell_idx] = data;
    }
  }
  return view;
}
