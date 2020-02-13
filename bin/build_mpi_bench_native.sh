#!/bin/bash

set -e

source toolchain/native_gcc.sh

pushd third-party/mpi-benchmarks

# Make sure we can find the MPI implementation
export CFLAGS="$CFLAGS -I/usr/local/faasm/openmpi/include"
export CXXFLAGS="$CXXFLAGS -I/usr/local/faasm/openmpi/include"
export LDFLAGS="-L/usr/local/faasm/openmpi/lib -lmpi"

make -j IMB-MPI1

popd
