#!/bin/bash

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

# -----------------------------------------
# Pulls the /usr/local/faasm directory from the current Faasm CLI
# -----------------------------------------

while getopts :d option
do
case "${option}"
in
d) CLEAN="ON";;
esac
done

pushd ${PROJ_ROOT} >> /dev/null

FAASM_VERSION=$(cat VERSION)
CPP_VERSION=$(cat clients/cpp/VERSION)
PY_VERSION=$(cat clients/python/VERSION)

IMAGE_TAG=faasm/cli:${FAASM_VERSION}
CPP_IMAGE_TAG=faasm/cpp-sysroot:${CPP_VERSION}
PY_IMAGE_TAG=faasm/cpython:${PY_VERSION}

# This path needs to be absolute with no ..s
TARGET_DIR=$(pwd)/dev/faasm-local
SOURCE_DIR=/usr/local/faasm

if [ -z $CLEAN ]; then
    echo "Not cleaning existing dir"
else
    # We use docker to clear out the existing dir so that it has the right perms
    echo "Cleaning existing dir"
    docker run -i\
        -v ${TARGET_DIR}:/tmp/localcp \
        ${IMAGE_TAG} \
        /bin/bash -c "rm -rf /tmp/localcp/*"
fi

# This function mounts the faasm-local dir from this repo to a temp location,
# then copies in the contents of /usr/local/faasm from the relevant container
function pop_local {
    echo "Populating ${TARGET_DIR} from ${SOURCE_DIR} in $1"
    docker run -i\
        -v ${TARGET_DIR}:/tmp/localcp \
        $1 \
        /bin/bash -c "cp -r /usr/local/faasm/* /tmp/localcp/"
}

pop_local ${IMAGE_TAG}
pop_local ${CPP_IMAGE_TAG}
pop_local ${PY_IMAGE_TAG}

popd >> /dev/null
