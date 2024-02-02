#!/bin/bash

set -o pipefail

# FIXME(830)
exit 0

# Skip python tests in WAMR
if [ "${FAASM_WASM_VM}" == "wamr" ]; then
    exit 0
fi

# Compile upload and execute a python function
faasmctl cli.python --cmd "./bin/inv_wrapper.sh cpython.func cpython.upload func.uploadpy hello func.invoke python hello" | tee output_1.log

# Modify the hello.cpp function
faasmctl cli.python --cmd "sed -i 's/Hello/Bye/g' ./func/python/hello.py"

# Flush the executors
faasmctl flush.workers

# Upload and invoke the function again
faasmctl cli.python --cmd "./bin/inv_wrapper.sh func.uploadpy hello func.invoke python hello" | tee output_2.log

# Finally, invoke it a third time
faasmctl cli.python --cmd "./bin/inv_wrapper.sh func.uploadpy hello func.invoke python hello" | tee output_3.log

# The first and second outputs must differ, error otherwise
cmp output_1.log output_2.log && exit 1

# FIXME: The second and the thid must be equal, error otherwise
# cmp output_2.log output_3.log || exit 1
