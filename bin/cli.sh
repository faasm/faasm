#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} > /dev/null

docker-compose -f docker-compose-cli.yml run cli /bin/bash

popd > /dev/null
