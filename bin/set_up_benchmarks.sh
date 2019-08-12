#!/bin/bash

set -e

WORKING_DIR=${HOME}/faasm/bench
SOURCE_DIR=/usr/local/code/faasm

mkdir -p ${WORKING_DIR}
pushd ${WORKING_DIR}

# Prepare out of tree build
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_C_COMPILER=/usr/bin/clang \
    -DCMAKE_BUILD_TYPE=Release \
    /usr/local/code/faasm

# Build benchmarks
cmake --build . --target codegen -- -j
cmake --build . --target bench_mem -- -j
cmake --build . --target bench_time -- -j
cmake --build . --target thread_bench_mem -- -j
cmake --build . --target thread_bench_time -- -j

# Generate codegen
./bin/codegen /usr/local/code/faasm/wasm/demo/noop
./bin/codegen /usr/local/code/faasm/wasm/demo/sleep

popd

pushd /usr/local/code/faasm

# Build docker image
docker build -t faasm/noop -f docker/noop.dockerfile .

popd
