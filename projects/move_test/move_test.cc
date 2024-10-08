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

#include "move_test.h"

#pragma hls_top
coord entrypoint(coord position, unsigned char dir) {
  // Case 1
  unsigned char delta_x = 0, delta_y = 0;
  switch (dir) {
    case UP:
      delta_y=-1;
      break;
    case DOWN:
      delta_y=1;
      break;
    case LEFT:
      delta_x=-1;
      break;
    case RIGHT:
      delta_x=1;
      break;
  }
  unsigned char x = position.x+delta_x;
  unsigned char y = position.y+delta_y;
  return coord{x = x, y = y};

  /*
  // Case 2
  unsigned char x = position.x, y = position.y;
  switch (dir) {
    case UP:
      y -= 1;
      break;
    case DOWN:
      y += 1;
      break;
    case LEFT:
      x -= 1;
      break;
    case RIGHT:
      x += 1;
      break;
  }
  return coord{x = x, y = y};
  */
}
