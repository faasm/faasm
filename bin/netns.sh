#!/bin/bash

set -e

if [ "$NETNS_MODE" == "off" ];
then
    echo "Network namespace support is off"
    exit 0
fi

if [ -z "$1" ]; then
    echo "Must provide a required number of namespaces"
    exit 1
fi

MAX_NET_NAMESPACES=$1;

python3 faasmcli/faasmcli/tasks/network.py ${MAX_NET_NAMESPACES}
