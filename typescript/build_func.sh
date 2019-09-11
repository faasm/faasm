#!/bin/bash

set -e

if [[ -z $1 ]]; then
    echo "Must provide function name"
    exit 1
fi

FUNC_NAME=$1

THIS_DIR=$(dirname $(readlink -f $0))

ASC_BIN=${THIS_DIR}/node_modules/assemblyscript/bin/asc

# Debug
${ASC_BIN} assembly/${FUNC_NAME}.ts \
    -b build/${FUNC_NAME}.debug.wasm \
    -t build/${FUNC_NAME}.debug.wast \
    --sourceMap \
    --validate \
    --debug

# Optimized
${ASC_BIN} assembly/${FUNC_NAME}.ts \
    -b build/${FUNC_NAME}.wasm \
    -t build/${FUNC_NAME}.wast \
    --sourceMap \
    --validate \
    --optimize

