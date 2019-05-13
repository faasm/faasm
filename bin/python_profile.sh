#!/bin/bash

set -e

perf record -k 1 ./cmake-build-debug/bin/func_runner demo fibonacci 42

perf inject -i perf.data -j -o perf.data.jitted

perf report -i perf.data.jitted
