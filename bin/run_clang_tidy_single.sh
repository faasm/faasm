#!/bin/bash

set -e

# TODO
# 1. Finish config file
# 2. Actually apply changes (add -fix flag)
# 3. Format after fix? (add -format flag)
BUILD_PATH=${FAASM_ROOT}/faasm/build/
CONFIG=${FAASM_ROOT}/faasm/.clang-tidy
    #-config ${CONFIG} \

# Run clang-tidy on a single file
run-clang-tidy-10.py \
    -config '' \
    -p ${BUILD_PATH} \
    $1 

