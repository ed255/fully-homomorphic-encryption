#!/bin/sh

set -ex

project="frogzone"
func="apply_move"
# func="get_cell"
# func="get_five_cells"
# func="get_cross_cells"
# func="get_horizontal_cells"
# func="get_vertical_cells"

cp -r "/projects/${project}/" "transpiler/examples/prj_${project}"
cat "transpiler/examples/prj_${project}/get_cell_no_check.cc" >> "transpiler/examples/prj_${project}/${func}.cc"
bazel build --sandbox_debug --verbose_failures "//transpiler/examples/prj_${project}:${func}_rs_fhe_lib" || true
# bazel test //transpiler/examples/prj_${project}:${project}_test
mkdir -p "/projects/${project}/out"
cp "bazel-out/k8-opt/bin/transpiler/examples/prj_${project}/${func}_rs_fhe_lib.rs" \
	"/projects/${project}/out/"
chmod +w-x+X -R "/projects/${project}/out/"
chown 1000 -R "/projects/${project}/out/"

