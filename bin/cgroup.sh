#!/bin/bash

set -e

CGROUP_USER=${SUDO_USER:-$USER}
CGROUP_USER=${CGROUP_USER:-$(whoami)}

CGROUP=cpu:faasm

echo "Setting up cgroup ${CGROUP} for ${CGROUP_USER}"

cgcreate -t ${CGROUP_USER}:${CGROUP_USER} -a ${CGROUP_USER}:${CGROUP_USER} -g ${CGROUP}