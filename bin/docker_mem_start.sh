#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide number of workers"
    exit 1
fi

N_WORKERS=$1

echo "Running Docker mem with ${N_WORKERS}"

for (( i=0; i<$N_WORKERS; i++ ))
do
    echo "Spawning background Docker $i"

    # Start a long-running container in the background
    # docker run --network host -di faasm/noop /bin/sh &
    # docker run -di faasm/noop /bin/sh &
    docker run --network bench -di faasm/noop /bin/sh &
done

wait

