#!/bin/bash

# ------------------------------------
# This script gets injected into the deployable lambda zip to do the start-up
#
# The script that creates the original can be found here:
# https://github.com/awslabs/aws-lambda-cpp/blob/master/packaging/packager
# ------------------------------------

set -euo pipefail
export AWS_EXECUTION_ENV=lambda-cpp
exec $LAMBDA_TASK_ROOT/lib/ld-linux-x86-64.so.2 --library-path $LAMBDA_TASK_ROOT/lib $LAMBDA_TASK_ROOT/bin/worker-lambda ${_HANDLER}
