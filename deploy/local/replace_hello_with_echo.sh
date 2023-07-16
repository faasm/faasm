#!/bin/bash
set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

ECHO_WASM=/code/cpp/build/func/demo/echo.wasm
HELLO_WASM=/code/cpp/build/func/demo/hello.wasm
CP_BIN=/usr/bin/cp

faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func demo echo"
faasmctl cli.cpp --cmd "${CP_BIN} ${ECHO_WASM} ${HELLO_WASM}"
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.upload demo hello"

popd > /dev/null
