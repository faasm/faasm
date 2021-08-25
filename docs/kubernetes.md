# Faasm Kubernetes/ Knative integration

Faasm runs on K8s using [Knative](https://knative.dev/).

Faasm's K8s configuration can be found in the [k8s](../deploy/k8s) directory,
and the relevant parts of the Faasm CLI can be found in the [Knative
tasks](../tasks/knative.py).

Faasm assumes a K8s cluster is set up with
[`kubectl`](https://kubernetes.io/docs/tasks/tools/install-kubectl/) available
on the commandline, along with [`kn`](https://github.com/knative/client), the
Knative client.

If you don't have a cluster set up, see the instructions at the bottom of this
page.

## Installing Knative

Faasm requires a minimal install of [Knative
serving](https://knative.dev/docs/install/any-kubernetes-cluster/).

If your cluster doesn't already have Knative installed, you can run:

```bash
# Install
inv knative.install

# Check
kubectl get pods -n knative-serving
kubectl get pods -n istio-system
```

## Deplying Faasm to K8s

You can deploy Faasm as follows, matching the replicas to the number of nodes in
your k8s cluster:

```bash
inv knative.deploy --replicas=4
```

This might take a couple of minutes depending on the underlying cluster.

Check with:

```bash
kubectl -n faasm get pods --watch
```

### Config file

Once everything has started up, you can populate your `faasm.ini` file at the
root of this project. To do this, run:

```
# Update the file
inv knative.ini-file
```

## Uploading functions

Once you have configured your `faasm.ini` file, you can use the Faasm, CPP,
Python CLIs via the [`docker-compose-k8s.yml`](../docker-compose-k8s.yml) file
in this repo:

```
# Start the container (be sure to have stopped all other docker-compose stuff)
docker-compose -f docker-compose-k8s.yml run cpp-cli /bin/bash

# Compile and upload a function
inv func demo hello
inv func.upload demo hello --host=<upload_host>
```

# Troubleshooting

First off you need to check that everything else is running. List all pods in
all namespaces and check for any whose status is not `Running`:

```bash
kubectl get pods --all-namespaces
```

Provided everything looks normal, or you find a container with something wrong,
you can look at logs. E.g. for a Faasm worker:

```
# Find the faasm-worker-xxx pod
kubectl -n faasm get pods

# Tail the logs for a specific pod
kubectl -n faasm logs -f faasm-worker-xxx user-container

# Tail the logs for all containers in the deployment
# You only need to specify the max-log-requests if you have more than 5 containers
kubectl -n faasm logs -f -c user-container -l serving.knative.dev/service=faasm-worker --max-log-requests=<N_CONTAINERS>

# Get all logs from the given deployment (add a very large --tail)
kubectl -n faasm logs --tail=100000 -c user-container -l serving.knative.dev/service=faasm-worker --max-log-requests=10 > /tmp/out.log
```

## Isolation and privileges

Faasm uses namespaces and cgroups to achieve isolation, therefore containers
running Faasm workers need privileges that they don't otherwise have. The
current solution to this is to run containers in `privileged` mode. This may not
be available on certain clusters, in which case you'll need to set the following
environment vars:

```
CGROUP_MODE=off
NETNS_MODE=off
```

# K8s Cluster set-up

## Google Kubernetes Engine

To set up Faasm on [GKE](https://console.cloud.google.com/kubernetes) you can do
the following:

- Set up an account and the [Cloud SDK](https://cloud.google.com/sdk) ([Ubuntu
  quick start](https://cloud.google.com/sdk/docs/quickstart-debian-ubuntu)).
- Create a Kubernetes cluster **with Istio enabled and version >=v1.15**.
- Aim for >=4 nodes with more than one vCPU.
- Set up your local `kubectl` to connect to your cluster (click the "Connect"
  button in the web interface).
- Check things are working by running `kubectl get nodes`.
- Install Knative serving as described below.

## Azure Kubernetes Service

To set up Faasm on AKS, you can do the following:

- Set up an account and install the
  [`az`](https://docs.microsoft.com/en-us/cli/azure/install-azure-cli) client.
- Create a resource group with sufficient quota.
- Create an AKS cluster under said resource group.
- Aim for >=4 nodes with more than one vCPU.
- Set up the local `kubectl` via the `get credentials` command.
- Manually install istio [using
  `istioctl`](https://istio.io/latest/docs/setup/getting-started)
- Continue with Faasm installation as described above.

## MicroK8s

Install according to the [official docs](https://microk8s.io/).

Note that you may have to set up different permissions to run without `sudo`
(although this is not necessarily required).

You should also enable `istio` with:

```bash
microk8s.enable istio
```

Finally, set up your `kubectl` to use this with:

```bash
rm -rf ~/.kube
mkdir ~/.kube
microk8s config > ~/.kube/config

# Check
kubectl get nodes
```

You can then continue with Faasm installation as described above.

## Bare metal

If you're deploying on a bare-metal cluster then you need to update the
`externalIPs` field in the `upload-service.yml` file to match your k8s master
node.

You also need to install Istio as described in [the Knative
docs](https://knative.dev/docs/install/any-kubernetes-cluster/).

### Redis bare-metal set-up

There are a couple of tweaks required to handle running Redis, as detailed in
the [Redis admin docs](https://redis.io/topics/admin).

First you can turn off transparent huge pages (add `transparent_hugepage=never`
to `GRUB_CMDLINE_LINUX` in `/etc/default/grub` and run `sudo update-grub`).

Then if testing under very high throughput you can set the following in
`etc/sysctl.conf`:

```
# Connection-related
net.ipv4.tcp_tw_recycle = 1
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_fin_timeout = 15
net.core.somaxconn = 65535

# Memory-related
vm.overcommit_memory=1
```
