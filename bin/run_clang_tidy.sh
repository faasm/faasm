#!/bin/bash

set -e

# TODO
# Actually apply changes (add -fix flag)
BUILD_PATH=${FAASM_ROOT}/build/
CONFIG=${FAASM_ROOT}/.clang-tidy

if [ -z "$1" ]; then
    TARGET_DIR="."
else
    TARGET_DIR=$1
fi

# Run clang-tidy on a set of files
run-clang-tidy-10.py \
    -header-filter '.*' \
    -format \
    -style 'file' \
    -config '' \
    -p ${BUILD_PATH} \
    $1 

