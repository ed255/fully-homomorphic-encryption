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

#ifndef FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_EXAMPLES_FIBONACCI_FIBONACCI_H_
#define FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_EXAMPLES_FIBONACCI_FIBONACCI_H_

#define NUM_OBJS 10

struct CellData {
    unsigned char id;
    unsigned char hp_atk;
    // unsigned char atk;
    bool is_consumed;

    CellData() {
        id = 0;
        hp_atk = 0;
        // atk = 0;
        is_consumed = 0;
    }
};

struct Object {
    unsigned char x;
    unsigned char y;
    CellData data;
};

struct State {
    Object objects[NUM_OBJS];
};

struct View {
    CellData cells[25];

    View& operator=(const View& b) {
        #pragma hls_unroll yes
        for(int i=0;i<25;i++) {
            cells[i] = b.cells[i];
        }
        return *this;
    }

    View() {
        #pragma hls_unroll yes
        for(int i=0;i<25;i++) {
            cells[i] = CellData();
        }
    }
};

View entrypoint(State state, unsigned char x, unsigned char y);

#endif  // FULLY_HOMOMORPHIC_ENCRYPTION_TRANSPILER_EXAMPLES_FIBONACCI_FIBONACCI_H_
