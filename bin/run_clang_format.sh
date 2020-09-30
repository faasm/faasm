#!/bin/bash

set -e

if [ -z "$1" ]; then
    TARGET_DIR="."
else
    TARGET_DIR=$1
fi

pushd ${TARGET_DIR} >> /dev/null

# Find all source files using Git to automatically respect .gitignore
clang-format-10 -i $(git ls-files "*.h" "*.cpp" "*.c") 

popd >> /dev/null

