#!/bin/bash

set -e

CGROUP_USER=${SUDO_USER:-$USER}
CGROUP_USER=${CGROUP_USER:-$(whoami)}

CGROUP_NAME=cpu:faasm

if [ "$CGROUP_MODE" == "off" ];
then
    echo "CGroup support is off"
    exit 0
fi

CGROOT=/sys/fs/cgroup
CGDIR="${CGROOT}/${CGROUP_NAME}"

# Ensure cgroups v2
if [[ ! -f "${CGROOT}/cgroup.controllers" ]]; then
    echo "cgroups v2 not detected at ${CGROOT} (missing cgroup.controllers)" >&2
    exit 1
fi

echo "Setting up cgroup ${CGROUP_NAME} for ${CGROUP_USER}"

# Enable the CPU controller on the parent (v2 requirement)
if grep -qw cpu "${CGROOT}/cgroup.controllers" \
    && ! grep -qw "+cpu" "${CGROOT}/cgroup.subtree_control" 2>/dev/null; then
    echo +cpu > "${CGROOT}/cgroup.subtree_control"
fi

# Create the cgroup
mkdir -p "${CGDIR}"
chown -R "${CGROUP_USER}:${CGROUP_USER}" "${CGDIR}"

echo "Done. cgroup: ${CGDIR}"
echo "To join from that user:  echo \$\$ > ${CGDIR}/cgroup.procs"
