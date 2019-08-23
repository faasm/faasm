#!/bin/bash

set -e

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Must provide prefix and number"
    exit 1
fi

PREFIX=$1
NUMBER=$2
NAMESPACE=${PREFIX}ns${NUMBER}
VIF_NAME=${PREFIX}${NUMBER}
VIF_PEER=${PREFIX}p${NUMBER}

echo "-----------------------------"
echo "Traffic shaping qdisc"
echo "-----------------------------"
echo ""
echo "---- Outside ----"
tc qdisc show dev ${VIF_NAME}

echo ""
echo "---- Inside ----"
sudo ip netns exec ${NAMESPACE} tc qdisc show dev ${VIF_PEER}

echo ""
echo "-----------------------------"
echo "Traffic shaping rate limit"
echo "-----------------------------"
echo ""

echo "---- Outside ----"
tc class show dev ${VIF_NAME}

echo ""
echo "---- Inside ----"
sudo ip netns exec ${NAMESPACE} tc class show dev ${VIF_PEER}
