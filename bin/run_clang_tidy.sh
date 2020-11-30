#!/bin/bash

set -e

BUILD_PATH=/build/faasm/
CONFIG=${FAASM_ROOT}/.clang-tidy

FILES=$(git ls-files {func,libs,src,tests}/{"*.h","*.cpp","*.c"})

if [ -z "$1" ]; then
    :
else
    pushd $1 >> /dev/null
    FILES=$(git ls-files {"*.h","*.cpp","*.c"})
fi

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
    ${FILES} > /dev/null

popd >> /dev/null || true

