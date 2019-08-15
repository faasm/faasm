# Set-up and usage

Faasm can be deployed to Kubernetes to create a distributed set-up. The configuration files are found in the `k8s`
directory. This has the following components:

- A load balancer handling incoming calls
- Multiple `edge` pods
- A `redis-queue` pod
- A `redis-state` pod
- Multiple `worker` pods
- An `upload` pod
- A system-wide `ConfigMap` to hold environment variables

## Using a Kubernetes deployment

Once deployed, the quickest way to play with a Kubernetes deployment is to SSH onto the master node and get the URL for
the `edge` and `upload` services:

```
kubectl get service edge --namespace=faasm
kubectl get service upload --namespace=faasm
```

# Isolation and privileges

Faasm uses namespaces and cgroups to achieve isolation, therefore containers running Faasm workers need privileges that they don't otherwise have. The current solution to this is to run containers in `privileged` mode. This gives the containers a lot of privileges on the host machine, something that could be improved in future (i.e. selecting the minimal set of privileges and only applying them).

## Docker host set-up

### Redis-related set-up

There are a couple of tweaks required to handle running Redis, as detailed in the [Redis admin docs](https://redis.io/topics/admin).

First you can turn off transparent huge pages (add `transparent_hugepage=never` to `GRUB_CMDLINE_LINUX` in `/etc/default/grub` and run `sudo update-grub`).

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
