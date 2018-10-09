#!/bin/bash

set -e;

# Move to right directory
ANSIBLE_DIR=/faasm/code/ansible
pushd ${ANSIBLE_DIR} >> /dev/null

echo "Setting up cgroup";
ansible-playbook cgroup.yml

echo "Setting up namespaces"
ansible-playbook namespaces.yml

popd >> /dev/null

# Continue with normal Docker execution
exec "$@"