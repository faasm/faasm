#!/bin/bash

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

export FAASM_BUILD_MOUNT=/build/faasm

RETURN_VAL=0

# Run the test server in the background
docker compose \
    up \
    -d \
    dist-test-server

# Run the tests
docker compose \
    run \
    --rm \
    faasm-cli \
    /build/faasm/bin/dist_tests

RETURN_VAL=$?

echo "-------------------------------------------"
echo "                SERVER LOGS                "
echo "-------------------------------------------"
docker compose logs dist-test-server

# Stop everything
docker compose stop

popd >> /dev/null

exit $RETURN_VAL
