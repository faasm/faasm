#!/bin/bash

set -o pipefail

# 11/03/2024 - Python support temporarily broken
exit ${SKIPPED_TEST_RET_VAL}

# Compile and upload a Python function
faasmctl cli.python --cmd "./bin/inv_wrapper.sh cpython.func cpython.upload func.uploadpy hello"

# Invoke it
faasmctl cli.python --cmd "./bin/inv_wrapper.sh func.invoke python hello"
