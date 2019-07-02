#!/bin/bash

# Script for running a CI-like environment locally

set -e

THIS_DIR=$(dirname $(readlink -f $0))

pushd ${THIS_DIR}/.. > /dev/null

docker-compose -f test-compose.yml run tester /bin/bash

popd > /dev/null