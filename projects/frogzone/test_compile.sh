#!/bin/sh

set -ex

g++ apply_move.cc get_cell.cc main.cc -o main
./main
