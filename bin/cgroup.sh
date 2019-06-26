#!/bin/bash

set -e

CGROUP=cpu:faasm
CGROUP_USER=$(logname)
echo "Setting up cgroup ${CGROUP} for ${CGROUP_USER}"

cgcreate -t ${CGROUP_USER}:${CGROUP_USER} -a ${CGROUP_USER}:${CGROUP_USER} -g ${CGROUP}