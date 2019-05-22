#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))

BIN_A=${THIS_DIR}/../cmake-build-debug/bin/codegen
BIN_B=${THIS_DIR}/../cmake-build-release/bin/codegen
BIN_C=${THIS_DIR}/../build/bin/codegen

if [[ -d "$1" ]]; then
    DIRECTORY=$1
else
    echo "Could not find directory $1"
    exit 1
fi

# Work out where the codegen binary is
if [[ -f "$BIN_A" ]]; then
    CODEGEN=${BIN_A}
elif [[ -f "$BIN_B" ]]; then
    CODEGEN=${BIN_B}
elif [[ -f "$BIN_C" ]]; then
    CODEGEN=${BIN_C}
else
    echo "Could not find codegen binary"
    exit 1
fi

find ${DIRECTORY} | grep "\.so$" | xargs --max-lines=1 --max-procs=4 ${CODEGEN}

