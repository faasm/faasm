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

Once everything has started up, you can get the relevant URLs as follows (on the master node), then populate your
`~/faasm/faasm.ini` file as described below.

## Config file

To avoid typing in hostnames and ports over and over, you can populate a section of your `~/faasm/faasm.ini` file.
To get the values, run `./bin/knative_route.sh` which should print out something like:

```
[Kubernetes]
invoke_host = ...   # Usually the IP of your master node
inoke_port = ...    # E.g. 31380
upload_host = ...   # IP of the upload service
upload_port = ...   # Usually 8002
```

You can then copy-paste this.

## Uploading functions

Once you have the upload URL you can upload functions using the tasks in this repo.

```
source workon.sh

# C++ functions
# Note, --prebuilt uses the functions checked into the repo
inv upload --prebuilt <user> <func>

# All Python functions
inv upload-all --py
```

## Invoking functions

To invoke functions you need to provide the relevant host and port:

```
inv invoke <user> <func>
```

## Flushing Redis

When workers die or are killed, you'll need to clear the queue:

```
inv redis-clear-queue --knative
```

## Uploading and running native functions

For benchmarking we need to run the functions in a more "normal" serverless way (i.e. natively in a container). To build the relevant container:

```
inv build-knative-native <user> <function>
```

This will use a parameterised Dockerfile to create a container that runs the given function natively.

You can test locally with:

```
# Build the container
inv build-knative-native <user> <function> --host

# Start the container
inv knative-native-local <user> <function>

# Submit a request
curl http://localhost:8080
```

Once you're happy you can run the following on your machine with knative access:

```
inv deploy-knative-native <user> <function>

inv invoke --native <user> <function>
```

Given that these will scale from zero, it may take a while on the first invocation.

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

# Links/ Notes

- Knative tutorial - https://github.com/meteatamel/knative-tutorial
- Samples in different languages at: https://knative.dev/docs/serving/samples/

