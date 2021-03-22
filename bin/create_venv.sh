#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
PROJ_ROOT=${THIS_DIR}/..
PIP=${PROJ_ROOT}/venv/bin/pip3

function pip_cmd {    
    source ${PROJ_ROOT}/venv/bin/activate && ${PIP} "$@"
}

pushd ${PROJ_ROOT}

if [ ! -d "venv" ]; then
    python3 -m venv venv 
fi

pip_cmd install -U pip
pip_cmd install -U setuptools
pip_cmd install -r faasmcli/requirements.txt

pushd clients/cpp 
pip_cmd install -e .
popd

pushd faasmcli 
pip_cmd install -e .
popd

popd
