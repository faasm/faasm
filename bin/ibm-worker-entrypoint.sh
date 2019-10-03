#!/bin/bash

set -e

pushd /usr/local/code/faasm >> /dev/null

# It seems like IBM Cloud Functions don't provide configurable environment variables.
# Therefore we need to set them here before staring the worker.

# CGroups/ network namespaces not supported
export CGROUP_MODE=off
export NETNS_MODE=off

# Redis instance is fixed as provisioned in IBM Cloud
export IBM_CLOUD_REDIS_HOST=
export REDIS_STATE_HOST=
export REDIS_QUEUE_HOST=

# Funciton storage using IBM cloud object store
export FUNCTION_STORAGE=ibm-cos

# Other config
export THREADS_PER_WORKER=20
export MAX_QUEUE_RATIO=1
export MAX_WORKERS_PER_FUNCTION=20
export GLOBAL_MESSAGE_TIMEOUT=300000
export BOUND_TIMEOUT=300000
export UNBOUND_TIMEOUT=500000

popd >> /dev/null

# Continue with normal command
exec "$@"