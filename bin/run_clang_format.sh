#!/bin/bash

set -e

function append_newline {
    if [[ -z "$(tail -c 1 "$1")" ]]; then
        :
    else
        echo >> "$1"
    fi
}

if [ -z "$1" ]; then
    TARGET_DIR="."
else
    TARGET_DIR=$1
fi

pushd ${TARGET_DIR} >> /dev/null

# Find all source files using Git to automatically respect .gitignore
FILES=$(git ls-files "*.h" "*.cpp" "*.c")

# Run clang-format
clang-format-10 -i ${FILES}

# Check newlines
for f in ${FILES}; do
    append_newline $f
done

popd >> /dev/null
