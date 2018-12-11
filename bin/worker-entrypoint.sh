#!/bin/bash

set -e

pushd /faasm/code >> /dev/null

echo "Setting up cgroup"
./bin/cgroup.sh

echo "Setting up namespaces"
./bin/netns.sh

popd >> /dev/null

# Continue with normal command
exec "$@"