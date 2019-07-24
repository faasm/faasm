#!/bin/bash

set -e

N_WORKERS=$1
N_ITERATIONS=$2

echo "Running Docker noop with ${N_WORKERS} for ${N_ITERATIONS}"

for i in {1..${N_ITERATIONS}}
do
    echo "Docker iteration $i"
    docker run faasm/noop /faasm/noop
done

