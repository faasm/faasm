#!/bin/bash

set -e

N_ITERATIONS=$1

echo "Running Docker noop with ${N_ITERATIONS} iterations"

for (( i=0; i<$N_ITERATIONS; i++ ))
do
    echo "Running Docker noop $i"
    docker run faasm/noop /faasm/noop
done

