## Network Isolation

To isolate networking we can user a combination of network namespaces, virtual interfaces and standard Linux traffic shaping tools. 

Each function (the `N`th on the system):

- Executes in its own network namespace `faasmnsN`
- This namespace has an associated pair of virtual interfaces `faasmN`/ `faasmpN` (one inside handling ingress/egress and one outside handling egress/ingress)
- Both interfaces have traffic shaping rules set with `tc`

The traffic shaping is possible because all kinds of traffic are seen as _egress_ on one of the interfaces:

```
# Egress (i.e. egress on veth_peer and ingress on veth)
user_code -> veth_peer -> | namespace | -> veth -> eth0 -> network

# Ingress (i.e. egress on veth and ingress on veth_peer)
user_code <- veth_peer <- | namespace | <- veth <- eth0 <- network
```

## Testing

We set a reasonable limit in the traffic shaping rules (>1MiB) so running a single test may not show the limiting in action. We need to run them in parallel to check it.

To run tests on both ingress and egress we need two remote machines, each with `iperf3` installed (_not_ `iperf`).

Note that both must have _at least one public nameserver listed in `/etc/resolv.conf`_, e.g.

```
nameserver   8.8.8.8
```

We can pick several namespaces, e.g. `faasmns1`, `faasmns2`, `faasmns3` and check the `tc` rules are the same for each:

```
# Check ingress qdisc (i.e. egress on interface outside the namespace)
sudo tc qdisc show dev faasm1
# Check rate limit
sudo tc class show dev faasm1

# Check egress qdisc (inside the namespace)
sudo ip netns exec faasmns1 tc qdisc show dev faasmp1
# Check rate limit
sudo ip netns exec faasmns1 tc class show dev faasmp1
```

You then need to set up an `iperf3` server on one of the hosts:

```
# Get the ip
ip addr

# Run the server
iperf3 -s
```

Then from the client host you can run the script from this repo:

```
# Root required for changing network namespace
sudo ./bin/network_check.sh <server_ip>
```

### egress

On the client:

```
iperf3 -c <server_ip>
```

This shows something like:

```
Connecting to host xxx.xxx.xxx.xxx, port 5201
[  4] local xxx.xxx.xxx.xxx port 39100 connected to xxx.xxx.xxx.xxx port 5201
[ ID] Interval           Transfer     Bandwidth       Retr  Cwnd
[  4]   0.00-1.00   sec  85.3 MBytes   715 Mbits/sec    0    375 KBytes       
[  4]   1.00-2.00   sec  84.9 MBytes   712 Mbits/sec    0    390 KBytes       
[  4]   2.00-3.00   sec  87.7 MBytes   736 Mbits/sec    0    390 KBytes       
[  4]   3.00-4.00   sec  90.4 MBytes   758 Mbits/sec    0    390 KBytes       
[  4]   4.00-5.00   sec  88.3 MBytes   741 Mbits/sec    0    390 KBytes       
```

Now we need to run _multiple simultaneous_ tests from within the namespaces using the script in this repo:

```

sudo ip netns exec faasmns1 iperf3 -c 146.179.131.175 > /tmp/ns1.out &; \
sudo ip netns exec faasmns2 iperf3 -c 146.179.131.175 > /tmp/ns2.out &; \
sudo ip netns exec faasmns3 iperf3 -c 146.179.131.175 > /tmp/ns3.out &; \
```

And get the following (much slower):

```
Connecting to host xxx.xxx.xxx.xxx, port 5201
[  4] local 10.200.1.2 port 46510 connected to xxx.xxx.xxx.xxx port 5201
[ ID] Interval           Transfer     Bandwidth       Retr  Cwnd
[  4]   0.00-1.00   sec   395 KBytes  3.23 Mbits/sec    0   43.8 KBytes       
[  4]   1.00-2.00   sec   127 KBytes  1.04 Mbits/sec    0   43.8 KBytes       
[  4]   2.00-3.00   sec   127 KBytes  1.04 Mbits/sec    0   43.8 KBytes       
[  4]   3.00-4.00   sec   127 KBytes  1.04 Mbits/sec    0   43.8 KBytes       
[  4]   4.00-5.00   sec  63.6 KBytes   521 Kbits/sec    0   43.8 KBytes   
```

### ingress

For ingress we can do the same trick but with the `-R` or `--reverse` option for `iperf3`. I.e.

```
iperf3 -R -c xxx.xxx.xxx.xxx
```

Gives 

```
Connecting to host xxx.xxx.xxx.xxx, port 5201
Reverse mode, remote host xxx.xxx.xxx.xxx is sending
[  4] local 146.169.26.164 port 39110 connected to xxx.xxx.xxx.xxx port 5201
[ ID] Interval           Transfer     Bandwidth
[  4]   0.00-1.00   sec   112 MBytes   937 Mbits/sec                  
[  4]   1.00-2.00   sec   112 MBytes   941 Mbits/sec                  
[  4]   2.00-3.00   sec   112 MBytes   941 Mbits/sec                  
[  4]   3.00-4.00   sec   112 MBytes   941 Mbits/sec                  
[  4]   4.00-5.00   sec   112 MBytes   941 Mbits/sec        
```

And from within the namespace:

```
sudo ip netns exec faasmns1 iperf3 -R -c xxx.xxx.xxx.xxx
```

Gives 

```
Connecting to host xxx.xxx.xxx.xxx, port 5201
Reverse mode, remote host xxx.xxx.xxx.xxx is sending
[  4] local 10.200.1.2 port 46530 connected to xxx.xxx.xxx.xxx port 5201
[ ID] Interval           Transfer     Bandwidth
[  4]   0.00-1.00   sec   164 KBytes  1.34 Mbits/sec                  
[  4]   1.00-2.00   sec   116 KBytes   950 Kbits/sec                  
[  4]   2.00-3.00   sec   115 KBytes   938 Kbits/sec                  
[  4]   3.00-4.00   sec   119 KBytes   973 Kbits/sec                  
[  4]   4.00-5.00   sec   117 KBytes   961 Kbits/sec                  
```