#!/bin/bash

set -e

# Run codegen
THIS_DIR=$(dirname $(readlink -f $0))
$THIS_DIR/entrypoint_codegen.sh

# Start hoststats
nohup hoststats start > /var/log/hoststats.log 2>&1 &

exec "$@"
