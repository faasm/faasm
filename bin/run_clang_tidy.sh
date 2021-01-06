#!/bin/bash

set -e

BUILD_PATH=/build/faasm/
CONFIG=${FAASM_ROOT}/.clang-tidy
DUMP_OUTPUT=true

# Function to actually run clang-tidy
function do_tidy {
    FILES="($@)"
    run-clang-tidy-10.py \
        -config '' \
        -j `nproc` \
        -fix \
        -format \
        -style 'file' \
        -p ${BUILD_PATH} \
        ${FILES}
}

# Support passing a file, directory or nothing
if [ -f "$1" ]; then
    echo "Running on file $1"
    FILES=($1)

    # Assume we want to see output if running on a single file
    unset DUMP_OUTPUT

elif [ -d "$1" ]; then
    pushd $1 >> /dev/null
    echo "Running on directory $1"
    FILES=$(git ls-files {"*.h","*.cpp","*.c"})

elif [ -z "$1" ]; then
    echo "Running on cwd at $(pwd)"
    FILES=$(git ls-files {func,libs,src,tests}/{"*.h","*.cpp","*.c"})

else
    echo "First argument must be a file or directory"
    exit 1
fi

# Dump output or not
if [ -z "$DUMP_OUTPUT" ]; then
    do_tidy ${FILES[@]}
else
    do_tidy ${FILES[@]} > /dev/null
fi

popd >> /dev/null || true

