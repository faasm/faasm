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

FILES=$(git ls-files {libs,func}/{"*.h","*.cpp","*.c"})

echo $FILES

# Run clang-tidy on a set of files
run-clang-tidy-10.py \
    -config '' \
    -j `nproc` \
    -fix \
    -format \
    -style 'file' \
    -p ${BUILD_PATH} \
    -extra-arg-before='-I/build/faasm/_deps/faabric_ext-src/include/faabric/mpi' \
    -quiet \
    ${FILES}

popd >> /dev/null

