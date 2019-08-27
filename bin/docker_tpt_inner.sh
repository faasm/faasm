#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide log file"
    exit 1
fi

log_file=$1

# Start timer
start_ms=$(date +%s%3N)

# Launch request
docker run -di faasm/noop /faasm/noop

# Time and log
now_ms=$(date +%s%3N)
elapsed_ms=$(( now_ms - start_ms ))

echo "${elapsed_ms} LATENCY" >> ${log_file}