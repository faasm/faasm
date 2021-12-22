#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..

# Set different virtual environment paths so that these don't clash when
# mounting the code in a development container
VENV_PATH="undetected"
if [[ -z "$FAASM_DOCKER" ]]; then
    VENV_PATH="${PROJ_ROOT}/venv-bm"
else
    VENV_PATH="/usr/local/code/faasm/venv"
fi

PIP=${VENV_PATH}/bin/pip3

function pip_cmd {
    source ${VENV_PATH}/bin/activate && ${PIP} "$@"
}

pushd ${PROJ_ROOT} >> /dev/null

if [ ! -d "venv" ]; then
    python3 -m venv ${VENV_PATH}
fi

pip_cmd install -U pip
pip_cmd install -U setuptools
pip_cmd install -r faasmcli/requirements.txt

pushd clients/cpp >> /dev/null
pip_cmd install -e .
popd >> /dev/null

pushd faasmcli >> /dev/null
pip_cmd install -e .
popd >> /dev/null

popd >> /dev/null
