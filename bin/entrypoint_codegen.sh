#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

OBJ_DIR="/usr/local/faasm/object"

ORIGINAL_FUNCTION_STORAGE=$FUNCTION_STORAGE
export FUNCTION_STORAGE=local
echo "Temporarily changing function storage from ${ORIGINAL_FUNCTION_STORAGE} to local"

# Run codegen for some functions if needed
if [[ "${SGD_CODEGEN}" == "on" ]]; then
  [ -f "$OBJ_DIR/sgd/reuters_svm/function.wasm.o" ] || ./bin/codegen_func sgd reuters_svm
fi

if [[ "${TF_CODEGEN}" == "on" ]]; then
  [ -f "$OBJ_DIR/tf/image/function.wasm.o" ] || ./bin/codegen_func tf image
fi

if [[ "${PYTHON_CODEGEN}" == "on" ]]; then
  if [[ ! -f "$OBJ_DIR/python/py_func/function.wasm.o"]]; then
    echo "Generating python object files"
    ./bin/codegen_func python py_func
    ./bin/codegen_shared_obj /usr/local/faasm/runtime_root/lib/python3.7
  fi
fi

echo "Reverting function storage from local to ${ORIGINAL_FUNCTION_STORAGE}"
export FUNCTION_STORAGE=${ORIGINAL_FUNCTION_STORAGE}

popd >> /dev/null

