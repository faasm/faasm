#!/bin/bash

set -e

ROOT=/usr/local/code/faasm

pushd ${ROOT}/pyodide/packages

make

popd

${ROOT}/bin/python_codegen.sh
${ROOT}/bin/set_up_python_root.sh
