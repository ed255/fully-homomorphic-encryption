# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# To build and run:
#
#   docker build -t google-fhe-transpiler -f docker/debian-bullseye.Dockerfile .
#   docker run --rm -i -t google-fhe-transpiler bash

FROM julianweng/fhe-google:latest

RUN apt-get update && apt-get install -y \
  vim \
  ripgrep \
  fd-find

WORKDIR /usr/src/fhe/

RUN git remote add jay https://github.com/Janmajayamall/fully-homomorphic-encryption
# RUN git pull jay compiler
# RUN git checkout jay/compiler
RUN git pull jay more_unroll
RUN git checkout jay/more_unroll
RUN git log -1
# Build an example to make sure all the intermediate build artifacts are
# compiled in the Docker image
RUN bazel build //transpiler/examples/fibonacci:fibonacci_rs_main
