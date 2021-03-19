#!/bin/bash

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

# -----------------------------------------
# Pulls the /usr/local/faasm directory from the current Faasm CLI
# -----------------------------------------

pushd ${PROJ_ROOT} >> /dev/null

FAASM_VERSION=$(cat ${PROJ_ROOT}/faasm/VERSION)
IMAGE_TAG=faasm/cli:${FAASM_VERSION}

# This path needs to be absolute with no ..s
TARGET_DIR=$(pwd)/faasm-local
SOURCE_DIR=/usr/local/faasm

# We use docker to clear out the existing dir so that it has the right perms
echo "Nuking existing dir"
docker run -i\
    -v ${TARGET_DIR}:/tmp/localcp \
    ${IMAGE_TAG} \
    /bin/bash -c "rm -rf /tmp/localcp/*"

# This command mounts the faasm-local dir from this repo to a temp location,
# then copies in the contents of /usr/local/faasm
echo "Populating ${TARGET_DIR} from ${SOURCE_DIR} in ${IMAGE_TAG}"
docker run -i\
    -v ${TARGET_DIR}:/tmp/localcp \
    ${IMAGE_TAG} \
    /bin/bash -c "cp -r /usr/local/faasm/* /tmp/localcp/"

popd >> /dev/null
