#!/bin/bash

set -e

IMAGE=$1
N_ITERATIONS=$2

echo "Running Docker faasm/${IMAGE} with ${N_ITERATIONS} iterations"

for (( i=0; i<$N_ITERATIONS; i++ ))
do
    echo "Running faasm/${IMAGE} $i"
    if [[ "$IMAGE" == "pynoop" ]]; then
      docker run faasm/pynoop
    else
      docker run faasm/${IMAGE} /faasm/noop
    fi
done
