#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
OUTPUT_DIR=${THIS_DIR}/../third-party/tensorflow/tensorflow/lite/tools/make/gen/linux_x86_64

if [[ -d "${OUTPUT_DIR}" ]]; then
    echo "Removing ${OUTPUT_DIR}"
    rm -rf ${OUTPUT_DIR}
else 
    echo "No directory found at ${OUTPUT_DIR}"
    exit 1
fi

