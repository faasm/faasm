#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide iperf3 server IP"
    exit 1
fi

SERVER_IP=$1

PREFIX=tester
NAMESPACE=testerns1
VIF_NAME=tester1
VIF_PEER=testerp1

EGRESS_LIMIT=20
INGRESS_LIMIT=10

THIS_DIR=$(dirname $(readlink -f $0))
INNER_SCRIPT=${THIS_DIR}/netns_inner.sh
CHECK_SCRIPT=${THIS_DIR}/netns_check.sh

# Create the namespace
${INNER_SCRIPT} ${PREFIX} 1 ${EGRESS_LIMIT} ${INGRESS_LIMIT} 300

# Run the check
${CHECK_SCRIPT} ${PREFIX} 1

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

echo ""
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