#!/bin/bash

set -e

source bin/workon.sh

perf record \
    -e cycles \
    -e sched:sched_switch --switch-events \
    --sample-cpu \
    -m 8M \
    --aio \
    --call-graph dwarf \
    ./dev/native/build/bin/func_runner lulesh func
