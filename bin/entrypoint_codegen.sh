#!/bin/bash

set -e

pushd /build/faasm >> /dev/null

OBJ_DIR="/usr/local/faasm/object"

ORIGINAL_FUNCTION_STORAGE=$FUNCTION_STORAGE
export FUNCTION_STORAGE=local
echo "Temporarily changing function storage from ${ORIGINAL_FUNCTION_STORAGE} to local"

# Run codegen for Python if needed
if [[ "${PYTHON_CODEGEN}" == "on" ]]; then
  if [[ ! -f "$OBJ_DIR/python/py_func/function.wasm.o" ]]; then
    echo "Generating python object files"
    ./bin/codegen_func python py_func
    ./bin/codegen_shared_obj /usr/local/faasm/runtime_root/lib/python3.8
  fi
fi

echo "Reverting function storage from local to ${ORIGINAL_FUNCTION_STORAGE}"
export FUNCTION_STORAGE=${ORIGINAL_FUNCTION_STORAGE}

popd >> /dev/null

