#!/bin/bash

set -e

THIS_DIR=$(dirname $(readlink -f $0))
INNER_SCRIPT=${THIS_DIR}/netns_inner.sh

if [ "$NETNS_MODE" == "off" ];
then
    echo "Network namespace support is off"
    exit 0
fi

if [ -z "$1" ]; then
    echo "Must provide a required number of namespaces"
    exit 1
fi

THREADS_PER_WORKER=$1;

EGRESS_KB=1024
INGRESS_KB=1024
IP_BASE=200
NAMESPACE_PREFIX=faasm

${INNER_SCRIPT} ${NAMESPACE_PREFIX} ${THREADS_PER_WORKER} ${EGRESS_KB} ${INGRESS_KB} ${IP_BASE}