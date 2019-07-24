#!/bin/bash

set -e

N_ITERATIONS=$1

echo "Running Docker noop for ${N_ITERATIONS} iterations"

for (( i=0; i<$N_ITERATIONS; i++ ))
do
    echo "Running Docker iteration $i"
    docker run faasm/noop /faasm/noop
done
