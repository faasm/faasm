#!/bin/bash

set -e

VERSION=$(cat VERSION)

docker pull faasm/testing:${VERSION}
docker pull faasm/redis:${VERSION}

