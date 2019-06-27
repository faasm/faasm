#!/bin/bash

# ------------------------------------
# This script gets injected into the deployable lambda zip to do the start-up
#
# The script that creates the original can be found here:
# https://github.com/awslabs/aws-lambda-cpp/blob/master/packaging/packager
#
# The FAASM_COMPONENT variable must be set
# ------------------------------------

set -eo pipefail
export AWS_EXECUTION_ENV=lambda-cpp

if [ -z "${FAASM_COMPONENT}" ]; then
    echo "Must set FAASM_COMPONENT env var to run"
    exit 1
fi

echo "Starting Faasm component $FAASM_COMPONENT"

# This line is copied as-is
FAASM_BINARY=$LAMBDA_TASK_ROOT/bin/$FAASM_COMPONENT-lambda
exec $LAMBDA_TASK_ROOT/lib/ld-linux-x86-64.so.2 --library-path $LAMBDA_TASK_ROOT/lib ${FAASM_BINARY} ${_HANDLER}
