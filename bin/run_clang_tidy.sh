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
    -config '' \
    -export-fixes clang_tidy_out.yml \
    -j `nproc` \
    -fix \
    -format \
    -style 'file' \
    -p ${BUILD_PATH} \
    -quiet \
    ${FILES}

popd >> /dev/null

