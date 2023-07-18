#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

# Run the function build and upload
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.user demo func.upload-user demo"
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.user mpi func.upload-user mpi"
faasmctl cli.cpp --cmd "./bin/inv_wrapper.sh func.user omp func.upload-user omp"

popd >> /dev/null
