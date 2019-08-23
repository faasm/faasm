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

THREADS_PER_WORKER=$1;
echo "Setting up ${THREADS_PER_WORKER} namespaces"

egress_rate=1mbit
egress_ceil=2mbit
ingress_rate=1mbit

# Enable IP forwarding on host
echo 1 > /proc/sys/net/ipv4/ip_forward

# Ensure there's a public nameserver in /etc/resolv.conf
echo "nameserver 8.8.8.8" >> /etc/resolv.conf

function setup_ns() {
    ns_name=faasmns$1
    vif_name=faasm$1
    vif_peer=faasmp$1        

    # Limit of 256 per byte of IP. Addresses in the given namespaces will be as follows:
    # n=[0,255]   -> 10.200.n.0
    # n=[256,511] -> 10.201.n.0
    # n=[512,767] -> 10.202.n.0
    # etc.
    part_one=$((200 + $1/256))
    part_two=$(($1 % 256))

    base_ip=10.$part_one.$part_two.0
    vif_ip=10.$part_one.$part_two.1
    vif_peer_ip=10.$part_one.$part_two.2

    ns_file="/var/run/netns/${ns_name}"
    if [ -f "$ns_file" ]
    then
        #echo "Namespace ${ns_name} already exists. Removing"
        #ip netns delete ${ns_name}
        echo "Namespace ${ns_name} already exists"
        return
    fi

    echo "Setting up namespace ${ns_name} (${base_ip})"

    # -------------------------
    # Namespace
    # -------------------------

    ip netns add ${ns_name}

    # -------------------------
    # Interfaces
    # -------------------------

    # Create veth link
    ip link add ${vif_name} type veth peer name ${vif_peer}

    # Add peer to namespace
    ip link set ${vif_peer} netns ${ns_name}

    # Set IP for virtual interface and bring up
    ip addr add ${vif_ip}/24 dev ${vif_name}
    ip link set ${vif_name} up

    # Same for peer (bring up loopback too)
    ip netns exec ${ns_name} ip addr add ${vif_peer_ip}/24 dev ${vif_peer}
    ip netns exec ${ns_name} ip link set ${vif_peer} up
    ip netns exec ${ns_name} ip link set lo up

    # Set up default route inside namespace
    ip netns exec ${ns_name} ip route add default via ${vif_ip}

    # -------------------------
    # iptables
    # -------------------------

    # Masquerading of subnet on eth0
    iptables -t nat -I POSTROUTING -s ${base_ip}/24 -o eth0 -j MASQUERADE

    # Forwarding rules
    iptables -I FORWARD -i eth0 -o ${vif_name} -j ACCEPT
    iptables -I FORWARD -o eth0 -i ${vif_name} -j ACCEPT

    # -------------------------
    # Traffic shaping
    # -------------------------

    # Rate limit egress on virtual interface
    # (effectively ingress for things inside the namespace)
    tc qdisc add dev ${vif_name} root handle 1: htb default 1
    tc class add dev ${vif_name} parent 1: classid 1:1 htb rate ${ingress_rate}

    # Rate limit egress on peer
    # (egress for things inside the namespace)
    ip netns exec ${ns_name} tc qdisc add dev ${vif_peer} root handle 1: htb default 1
    ip netns exec ${ns_name} tc class add dev ${vif_peer} parent 1: classid 1:1 htb rate ${egress_rate} ceil ${egress_ceil}
}

for (( i=1; i<=THREADS_PER_WORKER; i++ ))
do
    setup_ns ${i}
done
