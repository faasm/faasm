#!/bin/bash

set -e

SRC_URL=https://github.com/numpy/numpy/releases/download/v1.17.4/numpy-1.17.4.tar.gz
WORK_DIR=$HOME/faasm/numpy-build
NUMPY_DIR=numpy-1.17.4

mkdir -p $WORK_DIR

pushd $WORK_DIR

if [[ -f "$NUMPY_DIR" ]]; then
  echo "Numpy already downloaded at $NUMPY_DIR"
else
  wget $SRC_URL
  tar -xf ${NUMPY_DIR}.tar.gz
  rm ${NUMPY_DIR}.tar.gz
fi

pushd $NUMPY_DIR

export BLAS=None
export LAPACK=None
export ATLAS=None

python3 setup.py build -j 3 install

popd
popd

