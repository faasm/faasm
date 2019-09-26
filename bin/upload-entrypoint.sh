#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

PYTHON_RUNTIME_ROOT=/usr/local/faasm/runtime_root/lib/python3.7

if [[ -f "${PYTHON_DIR}/function.wasm.o" ]]; then
  echo "Python object files already exist"
else
  echo "Generating object file for Python"
  ./bin/codegen_func python py_func
  ./bin/codegen_shared_obj ${PYTHON_RUNTIME_ROOT}
fi

popd >> /dev/null

# Continue with normal command
exec "$@"
