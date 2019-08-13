#!/bin/bash

set -e

echo "---------------------"
echo "Building musl"
echo "---------------------"
./bin/build_musl.sh

echo "---------------------"
echo "Building libraries"
echo "---------------------"
inv compile-eigen
inv compile-malloc --clean
inv compile-libfaasm --clean
inv compile-libfake --clean
