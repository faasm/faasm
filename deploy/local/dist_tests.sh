#!/bin/bash


THIS_DIR=$(dirname $(readlink -f $0))
export PROJ_ROOT=${THIS_DIR}/../..

pushd ${PROJ_ROOT} > /dev/null

export FAASM_VERSION=$(cat VERSION)

# Set up image name
if [[ -z "${FAASM_CLI_IMAGE}" ]]; then
    export FAASM_CLI_IMAGE=faasm/cli:${FAASM_VERSION}
fi

INNER_SHELL=${SHELL:-"/bin/bash"}
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
