#!/bin/sh

set -ex

project="$project"

cp -r "/projects/${project}" "transpiler/examples/prj_${project}"
bazel build "//transpiler/examples/prj_${project}:${project}_rs_main"
mkdir -p "/projects/${project}/out"
cp "bazel-out/k8-opt/bin/transpiler/examples/prj_${project}/${project}_rs_fhe_lib.rs" \
	"/projects/${project}/out/"

