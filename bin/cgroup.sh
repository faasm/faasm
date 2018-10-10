#!/bin/bash

set -e

CGROUP=cpu:faasm
echo "Setting up cgroup ${CGROUP}"

cgcreate -t ${USER}:${USER} -a ${USER}:${USER} -g ${CGROUP}