#!/bin/bash

set -e

mkdir -p /tmp/curlfun
pushd /tmp/curlfun

emcc -g /usr/local/code/faasm/func/function_curl.c -I /tmp/curl-curl-7_61_0/include /tmp/curl-curl-7_61_0/lib/.libs/libcurl.so

mkdir -p /usr/local/code/faasm/wasm/simon/curl
mv a.out.wasm /usr/local/code/faasm/wasm/simon/curl/function.wasm
mv a.out.wast /usr/local/code/faasm/wasm/simon/curl/function.wast

popd
