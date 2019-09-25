#!/bin/bash

set -e

pushd /faasm/build >> /dev/null

PYTHON_DIR=/usr/local/code/faasm/wasm/python/py_func

if [[ -f "${PYTHON_DIR}/function.wasm.o" ]]; then
  echo "Python object file already exists"
else
  echo "Generating object file for Python"
  ./bin/codegen ${PYTHON_DIR}
fi

popd >> /dev/null

# Continue with normal command
exec "$@"
