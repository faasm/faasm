#!/bin/bash

set -e

# Run codegen
THIS_DIR=$(dirname $(readlink -f $0))
$THIS_DIR/entrypoint_codegen.sh

# Set up isolation
pushd /usr/local/code/faasm >> /dev/null

echo "Setting up cgroup"
./bin/cgroup.sh

echo "Setting up namespaces"
./bin/netns.sh ${THREADS_PER_WORKER}

popd >> /dev/null

# Continue with normal command
exec "$@"