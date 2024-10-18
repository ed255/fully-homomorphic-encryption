#!/bin/sh

set -ex

g++ apply_move.cc \
    get_cell_no_check.cc \
    get_cell.cc \
    get_five_cells.cc \
    get_cross_cells.cc \
    get_horizontal_cells.cc \
    get_vertical_cells.cc \
    main.cc -o main
./main
