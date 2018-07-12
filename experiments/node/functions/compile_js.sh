#/bin/bash

set -e

emcc doubler.c -Os -s WASM=1
