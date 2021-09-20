#!/bin/bash

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

RETURN_VAL=0

# Run the tests directly
docker-compose \
    run \
    --rm \
    faasm-cli \
    /build/faasm/bin/dist_tests

RETURN_VAL=$?

echo "-------------------------------------------"
echo "                WORKER LOGS                "
echo "-------------------------------------------"
docker-compose logs worker

# Stop everything
docker-compose stop

popd >> /dev/null

exit $RETURN_VAL
