#!/bin/bash

set -e

pushd /usr/local/code/faasm

# Set up cgroup and namespaces
./bin/cgroup.sh
./bin/netns.sh 5

# Set up TF data
inv -r faasmcli/faasmcli tf.upload --local-copy

popd

echo "Execute tests"
/faasm/build/bin/tests
