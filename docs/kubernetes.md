# Set-up and usage

Faasm can be deployed to Kubernetes and Knative to create a distributed set-up. The configuration files are found in the `k8s` directory.

## Knative Set-up

Provided you have an accessible Kubernetes cluster (cloud provider, local Minikube, bare metal etc.) with Knative installed, you can deploy Faasm as follows.

If you're deploying on a bare-metal cluster then you need to update the `externalIPs` field in the `upload-service.yml` file to match your k8s master node. 

On a cloud provider you should be provided with an endpoint which will handle the external connectivity.

To deploy, you can run:

```
# Local
inv k8s-deploy --local

# Remote bare-metal (must be run on master node)
inv k8s-deploy --bare-metal

# IBM Cloud
inv k8s-deploy --ibm
```

Once everything has started up, you can get the relevant URLs as follows (on the master node):

```
./bin/knative_route.sh
```

From here you need to note the invocation IP and port.

## Uploading functions

Once you have the upload URL you can upload functions using the tasks in this repo.

```
# Note, --prebuilt uses the functions checked into the repo
source workon.sh
inv upload --host=<your k8s host> --prebuilt <user> <func>
```

## Invoking functions

Invoking functions is a little different to normal Faasm functions and looks like:

```
inv invoke --host=<your k8s host> --port=<your invoke port> <user> <func>
```

## Troubleshooting

To look at the logs for the faasm containers:

```
# Find the faasm-worker-xxx pod
kubectl --namespace=faasm get pods

# Get the logs for the user-container part and queue proxy
kubectl --namespace=faasm logs faasm-worker-xxx user-container
kubectl --namespace=faasm logs faasm-worker-xxx queue-proxy
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
