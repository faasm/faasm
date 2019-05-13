#!/bin/bash

set -e

perf record -k 1 ./cmake-build-debug/bin/python_bench bench_float.py

perf inject -i perf.data -j -o perf.data.jitted

perf report -i perf.data.jitted
