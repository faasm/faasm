#!/bin/bash

set -e

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Must provide iperf3 server IP and namespace number"
    exit 1
fi

SERVER_IP=$1

PREFIX=tester
EGRESS_LIMIT=20
INGRESS_LIMIT=10

NAMESPACE=testerns1
VIF_NAME=faasm1
VIF_PEER=faasmp1

THIS_DIR=$(dirname $(readlink -f $0))
INNER_SCRIPT=${THIS_DIR}/netns_inner.sh

# Create the namespaces
${INNER_SCRIPT} ${PREFIX} 3 ${EGRESS_LIMIT} ${INGRESS_LIMIT} 300

# Check ingress qdisc (i.e. egress on interface outside the namespace)
echo "-----------------------------"
echo "Traffic shaping qdisc"
echo "-----------------------------"
echo ""
echo "---- Outside ----"
tc qdisc show dev ${VIF_NAME}

echo ""
echo "---- Inside ----"
sudo ip netns exec ${NAMESPACE} tc qdisc show dev ${VIF_PEER}

echo "-----------------------------"
echo "Traffic shaping rate limit"
echo "-----------------------------"
echo ""

echo "---- Outside ----"
tc class show dev ${VIF_NAME}

echo ""
echo "---- Inside ----"
sudo ip netns exec ${NAMESPACE} tc class show dev ${VIF_PEER}

echo ""
echo "------------------------------------"
echo "Connection speed outside"
echo "------------------------------------"

echo ""
echo "---- Egress ----"
iperf3 -c ${SERVER_IP}

echo ""
echo "---- Ingress ----"
iperf3 -R -c ${SERVER_IP}

echo "------------------------------------"
echo "Connection speed inside"
echo "------------------------------------"
echo ""

echo ""
echo "---- Egress ----"
sudo ip netns exec ${NAMESPACE} iperf3 -c ${SERVER_IP}

echo ""
echo "---- Ingress ----"

sudo ip netns exec ${NAMESPACE} iperf3 -R -c ${SERVER_IP}

echo ""
echo "Done"