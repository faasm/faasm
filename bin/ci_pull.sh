#!/bin/bash

set -e

pushd /usr/local/code/faasm

VERSION=$(cat VERSION)

docker pull faasm/testing:${VERSION}
docker pull faasm/redis:${VERSION}

popd
