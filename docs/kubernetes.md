# Set-up and usage

Faasm can be deployed to Kubernetes and Knative to create a distributed set-up. The configuration files are found
in the `k8s` directory. This has the following components.

## Knative Set-up

Provided you have an accessible Kubernetes cluster (and can run `kubectl` to access it), you can deploy Faasm
as follows:

```
# Local (NodePorts for services)
./bin/knative_deploy.sh local

# Remote (LoadBalancers for service)
./bin/knative_deploy.sh remote
```

Once everything has started up, you can get the relevant URLs as follows:

```
./bin/knative_route.sh
```

## Uploading functions

Once you have the upload URL you can upload functions using the tasks in this repo (from your k8s master).

```
source workon.sh
inv upload --host=<your k8s host> <user> <func>
```

## Troubleshooting

To look at the logs for the faasm containers:

```
# Find the faasm-xxx pod
kubectl get pods --namespace=faasm

# Get the logs for the user-container part and queue proxy
kubectl logs faasm-xxx user-container --namespace faasm
kubectl logs faasm-xxx queue-proxy --namespace faasm
```

# Isolation and privileges

Faasm uses namespaces and cgroups to achieve isolation, therefore containers running Faasm workers need privileges
that they don't otherwise have. The current solution to this is to run containers in `privileged` mode. This may not
be available on certain clusters, in which case you'll need to set `CGROUP_MODE=off` and `NETNS_MODE=off` in the
container config.

## Redis-related set-up

There are a couple of tweaks required to handle running Redis, as detailed in the
[Redis admin docs](https://redis.io/topics/admin).

First you can turn off transparent huge pages (add `transparent_hugepage=never` to `GRUB_CMDLINE_LINUX`
in `/etc/default/grub` and run `sudo update-grub`).

Then if testing under very high throughput you can set the following in `etc/sysctl.conf`:

```
# Connection-related
net.ipv4.tcp_tw_recycle = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_fin_timeout = 15
net.core.somaxconn = 65535

# Memory-related
vm.overcommit_memory=1
```
