from invoke import task
from os.path import isfile
from subprocess import run
import sys


def format_ns_name(prefix, number):
    return "{}ns{}".format(prefix, number)


def format_vif_name(prefix, number):
    return "{}{}".format(prefix, number)


def format_vif_peer_name(prefix, number):
    return "{}p{}".format(prefix, number)


def _setup_single_ns(
    prefix, ip_base, number, ingress_rate, egress_rate, egress_ceil
):
    ns_name = format_ns_name(prefix, number)
    vif_name = format_vif_name(prefix, number)
    vif_peer = format_vif_peer_name(prefix, number)

    # Limit of 256 per byte of IP. Addresses in the given namespaces will be as follows:
    # n=[0,255]   -> 10.200.n.0
    # n=[256,511] -> 10.201.n.0
    # n=[512,767] -> 10.202.n.0
    # etc.

    part_one = ip_base + int(number / 256)
    part_two = number % 256

    base_ip = "10.{}.{}.0".format(part_one, part_two)
    vif_ip = "10.{}.{}.1".format(part_one, part_two)
    vif_peer_ip = "10.{}.{}.2".format(part_one, part_two)

    ns_file = "/var/run/netns/{}".format(ns_name)

    if isfile(ns_file):
        print("Namespace {} already exsits".format(ns_name))
        return

    print("Setting up namespace {} ({})".format(ns_name, base_ip))

    # Add namespace
    run("ip netns add {}".format(ns_name), shell=True, check=True)
    # Create veth link
    run(
        "ip link add {} type veth peer name {}".format(vif_name, vif_peer),
        shell=True,
        check=True,
    )
    # Add peer to namespace
    run(
        "ip link set {} netns {}".format(vif_peer, ns_name),
        shell=True,
        check=True,
    )
    # Set IP for virtual interface and bring up
    run(
        "ip addr add {}/24 dev {}".format(vif_ip, vif_name),
        shell=True,
        check=True,
    )
    run("ip link set {} up".format(vif_name), shell=True, check=True)
    # Same for peer (bring up loopback too)
    run(
        "ip netns exec {} ip addr add {}/24 dev {}".format(
            ns_name, vif_peer_ip, vif_peer
        ),
        shell=True,
        check=True,
    )
    run(
        "ip netns exec {} ip link set {} up".format(ns_name, vif_peer),
        shell=True,
        check=True,
    )
    run(
        "ip netns exec {} ip link set lo up".format(ns_name),
        shell=True,
        check=True,
    )
    # Set up default route inside namespace
    run(
        "ip netns exec {} ip route add default via {}".format(ns_name, vif_ip),
        shell=True,
        check=True,
    )
    # Masquerading of subnet on eth0
    run(
        "iptables -t nat -I POSTROUTING -s {}/24 -o eth0 -j MASQUERADE".format(
            base_ip
        ),
        shell=True,
        check=True,
    )
    # Forwarding rules
    run(
        "iptables -I FORWARD -i eth0 -o {} -j ACCEPT".format(vif_name),
        shell=True,
        check=True,
    )
    run(
        "iptables -I FORWARD -o eth0 -i {} -j ACCEPT".format(vif_name),
        shell=True,
        check=True,
    )
    # Rate limit egress on virtual interface
    # (effectively ingress for things inside the namespace)
    run(
        "tc qdisc add dev {} root handle 1: htb default 1".format(vif_name),
        shell=True,
        check=True,
    )
    run(
        "tc class add dev {} parent 1: classid 1:1 htb rate {}".format(
            vif_name, ingress_rate
        ),
        shell=True,
        check=True,
    )
    # Rate limit egress on peer
    # (egress for things inside the namespace)
    run(
        "ip netns exec {} tc qdisc add dev {} root handle 1: htb default 1".format(
            ns_name, vif_peer
        ),
        shell=True,
        check=True,
    )
    run(
        "ip netns exec {} tc class add dev {} parent 1: classid 1:1 htb rate {}\
                ceil {}".format(
            ns_name, vif_peer, egress_rate, egress_ceil
        ),
        shell=True,
        check=True,
    )


def do_create_ns(
    ns_prefix, max_net_namespaces, egress_kb, ingress_kb, ip_base
):
    egress_rate = "{}Mbit".format(egress_kb)
    ingress_rate = "{}Mbit".format(ingress_kb)
    egress_ceil = "2mbit"

    print("----- Creating namespaces -----\n")
    print("Prefix:  {}".format(ns_prefix))
    print("Count:   {}".format(max_net_namespaces))
    print("Egress:  {}".format(egress_rate))
    print("Ingress: {}".format(ingress_rate))
    print("Ips:     10.{}.xxx.0\n".format(ip_base))

    with open("/proc/sys/net/ipv4/ip_forward", "w") as ip_file:
        ip_file.write("1\n")

    # Ensure there's a public nameserver in /etc/resolv.conf
    with open("/etc/resolv.conf", "a") as resolv_file:
        resolv_file.write("nameserver 8.8.8.8\n")

    for i in range(int(max_net_namespaces)):
        _setup_single_ns(
            ns_prefix,
            int(ip_base),
            i + 1,
            ingress_rate,
            egress_rate,
            egress_ceil,
        )


@task(default=True)
def create_ns(
    ctx,
    max_net_ns,
    egress_kb=1024,
    ingress_kb=1024,
    ip_base=200,
    ns_prefix="faasm",
):
    """
    Create a given number of network namespaces
    """
    do_create_ns(ns_prefix, max_net_ns, egress_kb, ingress_kb, ip_base)

    print(
        "If you want the ns limit to persist, make sure to update the env. variable:"
    )
    print("export MAX_NET_NAMESPACES={}".format(max_net_ns))


def do_check_ns(prefix, number):
    ns_name = format_ns_name(prefix, number)
    vif_name = format_vif_name(prefix, number)
    vif_peer = format_vif_peer_name(prefix, number)

    print("-----------------------------")
    print("Traffic shaping qdisc")
    print("-----------------------------")

    print("\n---- Outside ----")
    run("tc qdisc show dev {}".format(vif_name), shell=True, check=True)

    print("\n---- Inside ----")
    run(
        "ip netns exec {} tc qdisc show dev {}".format(ns_name, vif_peer),
        shell=True,
        check=True,
    )

    print("\n-----------------------------")
    print("Traffic shaping rate limit")
    print("-----------------------------")

    print("\n---- Outside ----")
    run("tc class show dev {}".format(vif_name), shell=True, check=True)

    print("\n---- Inside ----")
    run(
        "ip netns exec {} tc class show dev {}".format(ns_name, vif_peer),
        shell=True,
        check=True,
    )


@task
def check_ns(ctx, prefix, number):
    """
    Check a network namespace by prefix and number
    """
    do_check_ns(prefix, number)


@task
def test_ns_limit(ctx, server_ip):
    """
    Run test to check limits are correctly setup
    """
    prefix = "tester"
    ns_name = "testerns1"

    egress_kb = 100
    ingress_kb = 200
    ip_base = 10

    do_create_ns(prefix, 1, egress_kb, ingress_kb, ip_base)
    do_check_ns(prefix, 1)

    print("/n------------------------------------")
    print("Connection speed outside")
    print("------------------------------------")

    print("\n---- Egress ----")
    run("iperf3 -c {}".format(server_ip), shell=True, check=True)

    print("\n---- Ingress ----")
    run("iperf3 -R -c {}".format(server_ip), shell=True, check=True)

    print("\n------------------------------------")
    print("Connection speed inside")
    print("------------------------------------")

    print("\n---- Egress ----")
    run(
        "ip netns exec {} iperf3 -c {}".format(ns_name, server_ip),
        shell=True,
        check=True,
    )

    print("\n---- Ingress ----")
    run(
        "ip netns exec {} iperf3 -R -c {}".format(ns_name, server_ip),
        shell=True,
        check=True,
    )

    print("\nDone")


if __name__ == "__main__":
    do_create_ns("faasm", sys.argv[1], 1024, 1024, 200)
