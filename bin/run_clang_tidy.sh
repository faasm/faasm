#!/bin/bash

set -e

# TODO
# Actually apply changes (add -fix flag)
BUILD_PATH=/build/faasm/
CONFIG=${FAASM_ROOT}/.clang-tidy

if [ -z "$1" ]; then
    TARGET_DIR="."
else
    TARGET_DIR=$1
fi

pushd ${TARGET_DIR} >> /dev/null

FILES=$(git ls-files "*.h" "*.cpp" "*.c")

# Run clang-tidy on a set of files
run-clang-tidy-10.py \
    -j `nproc` \
    -fix \
    -export-fixes clang_tidy_out.yml \
    -extra-arg-before='-I/build/faasm/_deps/faabric_ext-src/include/faabric/mpi' \
    -format \
    -style 'file' \
    -config '' \
    -p ${BUILD_PATH} \
    ${FILES}

popd >> /dev/null

