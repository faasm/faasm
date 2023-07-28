#!/bin/bash

set -o pipefail

# Compile and upload a CPP function
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func demo hello func.upload demo hello"

# Invoke it
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.invoke demo hello"
