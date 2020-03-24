#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
pushd ${THIS_DIR}/.. >> /dev/null
FAASM_VERSION=$(cat VERSION)

TARGET_DIR=/tmp/faasm-libs

TOOLCHAIN_TAR=faasm-toolchain-${FAASM_VERSION}.tar.gz
SYSROOT_TAR=faasm-sysroot-${FAASM_VERSION}.tar.gz

LIB_URL=https://github.com/lsds/Faasm/releases/download/v${FAASM_VERSION}

function download_tar {
  echo ${LIB_URL}/$1
  curl -L ${LIB_URL}/$1 --output $1
  tar -xf $1
  rm $1
}

# Set up Faasm libs

mkdir -p ${TARGET_DIR}
pushd ${TARGET_DIR} >> /dev/null

download_tar ${SYSROOT_TAR}
download_tar ${TOOLCHAIN_TAR}

popd >> /dev/null