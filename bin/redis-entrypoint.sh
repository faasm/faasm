#!/bin/sh

# This script updates the node config file with the current IP
# Based on this repo: https://github.com/sanderploegsma/redis-cluster
CLUSTER_CONFIG="/redis-nodes.conf"

if [ -f ${CLUSTER_CONFIG} ]; then
  if [ -z "${POD_IP}" ]; then
    echo "Unable to determine Pod IP address!"
    exit 1
  fi
  echo "Updating my IP to ${POD_IP} in ${CLUSTER_CONFIG}"
  sed -i.bak -e "/myself/ s/[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}/${POD_IP}/" ${CLUSTER_CONFIG}
fi

exec "$@"

