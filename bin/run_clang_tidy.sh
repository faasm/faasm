#!/bin/bash

set -e
set -x

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

BUILD_PATH=/build/faasm/

HEADERS=${PROJ_ROOT}/include

CONFIG_FILE=${PROJ_ROOT}/.clang-tidy
CONFIG="$(cat $CONFIG_FILE)"

DUMP_OUTPUT=true

# Function to actually run clang-tidy
function do_tidy {
    FILES="$@"
    clang-tidy-10 \
        --config "${CONFIG}" \
        --enable-check-profile \
        --header-filter="${HEADERS}/*" \
        --fix \
        -p=${BUILD_PATH} \
        ${FILES}
}

MODE="all"

# Support passing a file, directory or nothing
if [ -f "$1" ]; then
    MODE="file"
    echo "Running on file $1"
    FILES=($1)

    # Assume we want to see output if running on a single file
    unset DUMP_OUTPUT
elif [ -d "$1" ]; then
    MODE="dir"
    pushd $1 >> /dev/null
    echo "Running on directory $1"
    FILES=$(git ls-files {"*.h","*.cpp","*.c"})

elif [ -z "$1" ]; then
    MODE="all"
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

if [ "$MODE" == "dir" ]; then
    popd >> /dev/null || true
fi

