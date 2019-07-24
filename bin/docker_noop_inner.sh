#!/bin/bash

set -e

N_ITERATIONS=$1
ADD_SLEEP=$2
SLEEP_SECONDS=5

for (( i=0; i<$N_ITERATIONS; i++ ))
do
    if [[ $ADD_SLEEP > 0 ]]; then
        echo "Running Docker sleep $i"
        docker run faasm/noop sleep ${SLEEP_SECONDS}
    else
        echo "Running Docker noop $i"
        docker run faasm/noop /faasm/noop
    fi
done
