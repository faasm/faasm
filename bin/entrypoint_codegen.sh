#!/bin/bash

set -e

pushd /build/faasm >> /dev/null

OBJ_DIR="/usr/local/faasm/object"

# Run codegen for Python if needed
if [[ "${PYTHON_CODEGEN}" == "on" ]]; then
  if [[ ! -f "$OBJ_DIR/python/py_func/function.wasm.o" ]]; then
    echo "Generating python object files"
    ./bin/codegen_func python --func py_func
    ./bin/codegen_shared_obj /usr/local/faasm/runtime_root/lib/python3.8
  fi
fi

popd >> /dev/null

