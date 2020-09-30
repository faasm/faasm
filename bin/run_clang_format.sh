#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must specify a directory (use . for this dir)"
    exit 
fi

TARGET_DIR=$1
echo "Running clang-format on ${TARGET_DIR}"

pushd ${TARGET_DIR} >> /dev/null

# Find all .h or .cpp files we want to format
FILES=$(find . \( -name "*.h" -o -name "*.cpp" -o -name "*.c" \) \
    -not -path "./build/*" \
    -not -path "./third-party/*") 

for f in $FILES
do
    echo "Format $f"
    clang-format-10 -i $f
done

popd >> /dev/null
