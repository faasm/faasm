#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

PYTHON_DIR=/usr/local/code/faasm/wasm/python/py_func
PYTHON_RUNTIME_ROOT=/usr/local/faasm/runtime_root/lib/python3.7

if [[ -f "${PYTHON_DIR}/function.wasm.o" ]]; then
  echo "Python object file already exists"
else
  echo "Generating object file for Python"
  ./bin/codegen ${PYTHON_DIR}
  ./bin/codegen ${PYTHON_RUNTIME_ROOT}
fi

popd >> /dev/null

# Continue with normal command
exec "$@"
