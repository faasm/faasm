#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

# Set python dependencies
source ./bin/workon.sh

# Build everything required for the distributed tests
inv -r faasmcli/faasmcli dev.cmake --build=Debug
inv -r faasmcli/faasmcli dev.cc codegen_func
inv -r faasmcli/faasmcli dev.cc codegen_shared_obj
inv -r faasmcli/faasmcli dev.cc dist_tests
inv -r faasmcli/faasmcli dev.cc dist_test_server
inv -r faasmcli/faasmcli dev.cc planner_server
inv -r faasmcli/faasmcli dev.cc upload

popd >> /dev/null
