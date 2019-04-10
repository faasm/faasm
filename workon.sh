#!/bin/bash

source /usr/local/code/faasm/pyodide/emsdk/emsdk/emsdk_env.sh
export EMCC_WASM_BACKEND=1

WASM_BIN=/usr/local/code/faasm/pyodide/emsdk/emsdk/upstream/4854/bin
export PATH=${WASM_BIN}:${PATH}

if [ ! -d "venv" ]; then
    python3 -m venv venv
fi

export VIRTUAL_ENV_DISABLE_PROMPT=1
source venv/bin/activate

export PS1="(faasm) $PS1"
