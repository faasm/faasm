#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
INNER_SCRIPT=${THIS_DIR}/docker_noop_mem_inner.sh

N_WORKERS=$1

echo "Running Docker mem with ${N_WORKERS}"

for (( i=0; i<$N_WORKERS; i++ ))
do
    echo "Spawning background Docker $i"

    # This sleep has to be long enough to allow lots of containers to be started and remain running
    docker run faasm/noop sleep 30 &
done

wait

