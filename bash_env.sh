#!/bin/bash

source /usr/local/code/faasm/pyodide/emsdk/emsdk/emsdk_env.sh
export EMCC_WASM_BACKEND=1

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

source venv/bin/activate
