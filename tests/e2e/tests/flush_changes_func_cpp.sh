#!/bin/bash

set -o pipefail

# Cross-compile the demo/hello function, upload it to the server, and run it
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func demo hello func.upload demo hello func.invoke demo hello" | tee output_1.log

# Modify the hello.cpp function
faasmctl cli.cpp --cmd "sed -i 's/Hello/Bye/g' ./func/demo/hello.cpp"

# Flush the executors
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.flush"

# Cross-compile again the demo/hello function, upload it to the server, and run it
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func demo hello func.upload demo hello func.invoke demo hello" | tee output_2.log

# The first and second outputs must differ, error otherwise
cmp output_1.log output_2.log && exit 1
