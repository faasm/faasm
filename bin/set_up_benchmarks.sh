#!/bin/bash

set -e

sudo mkdir -p /usr/local/faasm
sudo chown -R ${USER}:${USER} /usr/local/faasm

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
cmake --build . --target codegen_func -- -j
cmake --build . --target codegen_shared_obj -- -j
cmake --build . --target bench_mem -- -j
cmake --build . --target bench_time -- -j
cmake --build . --target bench_tpt -- -j
cmake --build . --target thread_bench_mem -- -j
cmake --build . --target thread_bench_time -- -j
cmake --build . --target max_thread_experiment -- -j
cmake --build . --target max_mem_experiment -- -j
cmake --build . --target rlimit_experiment -- -j
cmake --build . --target poly_bench -- -j
cmake --build . --target python_bench -- -j
cmake --build . --target upload -- -j

# Run codegen for functions
./bin/codegen_func demo noop
./bin/codegen_func demo lock
./bin/codegen_func demo sleep_long
./bin/codegen_func demo sleep_short

popd

pushd /usr/local/code/faasm

# Build docker image
docker build -t faasm/noop -f docker/noop.dockerfile .

popd
