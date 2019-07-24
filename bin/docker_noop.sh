#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
INNER_SCRIPT=${THIS_DIR}/docker_noop_inner.sh

N_WORKERS=$1
N_ITERATIONS=$2

echo "Running Docker noop with ${N_WORKERS} workers for ${N_ITERATIONS}"

for (( i=0; i<$N_WORKERS; i++ ))
do
    echo "Spawning background Docker noop $i"
    ${INNER_SCRIPT} ${N_ITERATIONS} &
done

wait

