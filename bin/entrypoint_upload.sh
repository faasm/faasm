#!/bin/bash

set -e

# Run codegen
THIS_DIR=$(dirname $(readlink -f $0))
$THIS_DIR/entrypoint_codegen.sh

exec "$@"
