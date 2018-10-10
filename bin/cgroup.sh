#!/bin/bash

set -e

CGROUP=cpu:faasm
echo "Setting up cgroup ${CGROUP} for ${SUDO_USER}"

cgcreate -t ${SUDO_USER}:${SUDO_USER} -a ${SUDO_USER}:${SUDO_USER} -g ${CGROUP}