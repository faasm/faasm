#/bin/bash

set -e

emcc doubler.c -Os -s WASM=1 -s SIDE_MODULE=1 -o doubler.wasm
