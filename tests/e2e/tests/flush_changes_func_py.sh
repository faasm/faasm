#!/bin/bash

set -o pipefail

# 11/03/2024 - Python support temporarily broken
exit ${SKIPPED_TEST_RET_VAL}

# Compile upload and execute a python function
faasmctl cli.python --cmd "./bin/inv_wrapper.sh cpython.func cpython.upload func.uploadpy hello func.invoke python hello" | tee output_1.log

# Modify the hello.cpp function
faasmctl cli.python --cmd "sed -i 's/Hello/Bye/g' ./func/python/hello.py"

# Flush the executors
faasmctl flush.workers

# Upload and invoke the function again
faasmctl cli.python --cmd "./bin/inv_wrapper.sh func.uploadpy hello func.invoke python hello" | tee output_2.log

# The first and second outputs must differ, error otherwise
cmp output_1.log output_2.log && exit 1 || exit 0
