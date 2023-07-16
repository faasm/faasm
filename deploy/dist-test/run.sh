#!/bin/bash

THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..
pushd ${PROJ_ROOT} > /dev/null

RETURN_VAL=0

# Run the distributed tests
faasmctl cli.faasm --cmd "/build/faasm/bin/dist_tests"

RETURN_VAL=$?

echo "-------------------------------------------"
echo "                SERVER LOGS                "
echo "-------------------------------------------"
docker compose logs dist-test-server
faasmctl logs -s dist-test-server

popd >> /dev/null

exit $RETURN_VAL
