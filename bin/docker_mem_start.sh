#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide name of docker network"
    exit 1
fi
if [ -z "$2" ]; then
    echo "Must provide number of workers"
    exit 1
fi

DOCKER_NETWORK=$1
N_WORKERS=$2

echo "Running Docker mem with ${N_WORKERS} workers on network ${DOCKER_NETWORK}"

for (( i=0; i<$N_WORKERS; i++ ))
do
    echo "Spawning background Docker $i"

    # Start a long-running container in the background
    docker run --network ${DOCKER_NETWORK} -di faasm/noop /bin/sh &
done

wait

