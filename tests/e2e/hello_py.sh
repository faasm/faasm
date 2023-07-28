#!/bin/bash

set -o pipefail

# Skip python tests in WAMR
if [ "${WASM_VM}" == "wamr" ]; then
    exit 0
fi

# Compile and upload a Python function
faasmctl cli.python --cmd "./bin/inv_wrapper.sh cpython.func cpython.upload func.uploadpy hello"

# Invoke it
faasmctl cli.python --cmd "./bin/inv_wrapper.sh func.invoke python hello"
