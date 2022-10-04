#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

ECHO_WASM=/code/cpp/build/func/demo/echo.wasm
HELLO_WASM=/code/cpp/build/func/demo/hello.wasm
CP_BIN=/usr/bin/cp

docker compose run -T cpp bash -c \
    "./bin/inv_wrapper.sh func demo echo && ${CP_BIN} ${ECHO_WASM} ${HELLO_WASM} && ./bin/inv_wrapper.sh func.upload demo hello"

popd > /dev/null
