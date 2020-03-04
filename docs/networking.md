# Network Isolation

To isolate networking we can user a combination of network namespaces, 
virtual interfaces and standard Linux traffic shaping tools. 

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

### Quick check

To run a quick check on the Faasm namespaces you can run:

```
# Create ten namespaces
sudo ./bin/netns.sh 10

# Check one or two
sudo ./bin/netns_check.sh faasm 1
sudo ./bin/netns_check.sh faasm 5
```

You can then make sure the limits are set up properly.

### Limits test

To run a fuller test on both ingress and egress we need two remote machines, each with `iperf3` installed (_not_ `iperf`). Note that both must have _at least one public nameserver listed in `/etc/resolv.conf`_ e.g.:

```
nameserver   8.8.8.8
```

You then need to set up an `iperf3` server on one of the hosts:

```
# Get the ip
ip addr

# Run the server
iperf3 -s
```

The test itself can be run with the script in this repo on the other machine:

```
sudo ./bin/netns_limit_test.sh <server_ip>
```

Note that for the actual system we use a permissive limit on ingress/egress (>1MiB) so to run the test we need to create an artificially restrictive namespace.
