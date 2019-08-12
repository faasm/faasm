#!/bin/bash

PROJ_ROOT=$(dirname $(dirname $(readlink -f $0)))
ANSIBLE_DIR=${PROJ_ROOT}/ansible

pushd ${ANSIBLE_DIR}

ansible-playbook -i inventory/benchmark.yml benchmark.yml

popd
