#!/bin/bash

set -e

find $1 | grep "\.so$" | xargs --max-lines=1 --max-procs=4 /usr/local/code/faasm/cmake-build-debug/bin/codegen

