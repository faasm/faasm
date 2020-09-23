#!/bin/bash

set -e

# Find all .h or .cpp files we want to format
FILES=$(find . \( -name "*.h" -o -name "*.cpp" -o -name "*.c" \) \
    -not -path "./build/*" \
    -not -path "./third-party/*") 

for f in $FILES
do
    echo "Format $f"
    clang-format-10 -i $f
done

