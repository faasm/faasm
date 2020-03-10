# Faasm Kubernetes/ Knative integration

Faasm is a runtime designed to be integrated into other serverless platforms.
The recommended integration is with [Knative](https://knative.dev/).

All Kubernetes and Knative configuration can be found in the [k8s](../k8s) 
directory, and the [Knative tasks](../tasks/knative.py).

## Knative Set-up

Provided you have an accessible Kubernetes cluster (cloud provider, local Minikube, 
bare metal etc.) with Knative installed, you can deploy Faasm as follows.

If you're deploying on a bare-metal cluster then you need to update the `externalIPs` 
field in the `upload-service.yml` file to match your k8s master node. 

On a cloud provider you should be provided with an endpoint which will handle the external 
connectivity.

To deploy, you can run:

```
# Local
inv knative.deploy --local

# Remote bare-metal (must be run on master node)
inv knative.deploy
```

Once everything has started up, you can get the relevant URLs as follows (on the master node), 
then populate your `~/faasm/faasm.ini` file as described below.

## Config file

To avoid typing in hostnames and ports over and over, you can populate a section of your 
`~/faasm/faasm.ini` file. To get the values, run `./bin/knative_route.sh` which should print out 
something like:

```
[Faasm]
invoke_host = ...   # Usually the IP of your master node
inoke_port = ...    # E.g. 31380
upload_host = ...   # IP of the upload service
upload_port = ...   # Usually 8002
```

You can then copy-paste this.

## Uploading functions

Once you have configured your `~/faasm/faasm.ini` file, you can use the Faasm 
CLI as normal, e.g.

```
inv upload demo hello
inv invoke demo hello
```

## Flushing Redis

When workers die or are killed, you'll need to clear the queue:

```
inv redis.clear-queue --knative
```

## Uploading and running native functions

### C++

For benchmarking we need to run the functions in a more "normal" serverless way (i.e. natively 
in a container). To build the relevant container:

```
inv knative.build-native <user> <function>
```

This will use a parameterised Dockerfile to create a container that runs the given function 
natively. You can test locally with:

```
# Build the container
inv knative.build-native <user> <function> --nopush

# Start the container
inv knative.native-local <user> <function>

# Submit a request
inv invoke <user> <function> --native
```

Once you're happy you can run the following on your machine with knative access:

```
inv knative.deploy-native <user> <function>

inv invoke --native <user> <function>
```

**Note** For anything that requires chaining we must run it asynchronously so that things 
don't get clogged up. To do this:

```
inv invoke --native --poll <user> <function>
```

### Python

To run Python functions natively we use pyfaasm and a standard Flask-based knative Python 
executor. This can be found at `func/knative_native.py`. We can build the container with:

```
inv docker.build -c knative-native-python --push
```

To check things locally:

```
inv knative.native-python-local
inv invoke python hello --py
```

To deploy, from the machine with k8s access:

```
inv knative.deploy-native-python
```

## Troubleshooting

To look at the logs for the faasm containers:

```
# Find the faasm-worker-xxx pod
kubectl --namespace=faasm get pods

# Tail the logs for a specific pod
kubectl -n faasm logs -f faasm-worker-xxx user-container

# Tail the logs for all containers in the deployment
# You only need to specify the max-log-requests if you have more than 5 containers
kubectl -n faasm logs -f -c user-container -l serving.knative.dev/service=faasm-worker --max-log-requests=<N_CONTAINERS>

# Get all logs from the given deployment (add a very large --tail)
kubectl -n faasm logs --tail=100000 -c user-container -l serving.knative.dev/service=faasm-worker --max-log-requests=10 > /tmp/out.log
```

# Isolation and privileges

Faasm uses namespaces and cgroups to achieve isolation, therefore containers running 
Faasm workers need privileges that they don't otherwise have. The current solution to 
this is to run containers in `privileged` mode. This may not be available on certain 
clusters, in which case you'll need to set the following environment vars:
 
```
CGROUP_MODE=off
NETNS_MODE=off
```

## Redis-related set-up

There are a couple of tweaks required to handle running Redis, as detailed in the
[Redis admin docs](https://redis.io/topics/admin).

First you can turn off transparent huge pages (add `transparent_hugepage=never` 
to `GRUB_CMDLINE_LINUX` in `/etc/default/grub` and run `sudo update-grub`).

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
