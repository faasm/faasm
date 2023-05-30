#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

pushd ${PROJ_ROOT} >> /dev/null

if [[ -z "$FAASM_DOCKER" ]]; then
    VENV_PATH="${PROJ_ROOT}/venv-bm"
else
    VENV_PATH="${PROJ_ROOT}/venv"
fi

until test -f ${VENV_PATH}/faasm_venv.BUILT
do
   echo "Waiting for python virtual environment to be ready..."
   sleep 3
done

popd >> /dev/null
