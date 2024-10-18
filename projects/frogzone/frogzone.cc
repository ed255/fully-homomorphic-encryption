#include "frogzone.h"

#pragma hls_top
View entrypoint(State state, unsigned char x, unsigned char y) {
  struct View view;

  int i = 0;
  // int j = 0;
  // #pragma hls_unroll yes
  // for (int i=0; i<5; i++) {
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
  // }
  return view;
}
