#!/bin/bash

set -e

if [ -z "$1" ]; then
    echo "Must provide iperf3 server IP"
    exit 1
fi

SERVER_IP=$1

echo "------------------------------------"
echo "Normal connection speed"
echo "------------------------------------"

echo "---- Egress ----"
iperf3 -c ${SERVER_IP}

echo "---- Ingress ----"
iperf3 -R -c ${SERVER_IP}

echo "------------------------------------"
echo "Simultaneous network namespace tests"
echo "------------------------------------"
echo ""

sudo ip netns exec faasmns1 iperf3 -c ${SERVER_IP} > /tmp/ns1.out 2>&1 &
sudo ip netns exec faasmns2 iperf3 -c ${SERVER_IP} > /tmp/ns2.out 2>&1 &
sudo ip netns exec faasmns3 iperf3 -c ${SERVER_IP} > /tmp/ns3.out 2>&1 &

wait

echo "------------------------------------"
echo "Egress results"
echo "------------------------------------"

echo ""
echo "---- faasmns1 ----"
cat /tmp/ns1.out

echo ""
echo "---- faasmns2 ----"
cat /tmp/ns2.out

echo ""
echo "---- faasmns3 ----"
cat /tmp/ns3.out

echo "------------------------------------"
echo "Ingress results"
echo "------------------------------------"

sudo ip netns exec faasmns1 iperf3 -R -c ${SERVER_IP} > /tmp/ns1.out 2>&1 &
sudo ip netns exec faasmns2 iperf3 -R -c ${SERVER_IP} > /tmp/ns2.out 2>&1 &
sudo ip netns exec faasmns3 iperf3 -R -c ${SERVER_IP} > /tmp/ns3.out 2>&1 &

wait

echo ""
echo "---- faasmns1 ----"
cat /tmp/ns1.out

echo ""
echo "---- faasmns2 ----"
cat /tmp/ns2.out

echo ""
echo "---- faasmns3 ----"
cat /tmp/ns3.out
