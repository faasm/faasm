#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

# Set python dependencies
source ./bin/workon.sh

# Build everything required for the distributed tests
inv dev.conan --build=Debug
inv dev.cmake --build=Debug
inv dev.cc codegen_func
inv dev.cc codegen_shared_obj
inv dev.cc dist_tests
inv dev.cc dist_test_server
inv dev.cc planner_server
inv dev.cc upload

popd >> /dev/null
