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

## Installing Knative and Istio

Faasm requires the underlying K8s cluster to have both Knative Serving and Istio
installed.

To install both:

```bash
# NOTE: this often fails on the first run, just rerun and it should work
inv knative.install
```

You can check whether the pods are ready with:

```bash
kubectl get pods -n knative-serving
kubectl get pods -n istio-system
```

## Deplying Faasm to K8s

Once Knative and Istio are installed, you can deploy Faasm as follows:

```bash
# Deploy with Knative auto-scaling
inv knative.deploy

# Deploy with fixed number of replicas
inv knative.deploy --replicas=4
```

If specifying `replicas`, the max should usually be one less than the number
of K8s nodes. This leaves enough space for the scheduler to place one Faasm
worker on each K8s node, while still respecting resource constraints.

If Faasm workers are getting colocated on nodes, this means that there is not
enough space on the nodes that aren't being used, so you may need to reduce the
number of replicas.

The deploy can take up to a couple of minutes.

Once everything has started up, Faasm should also generate a config file,
`faasm.ini` at root of this project. This contains the relevant endpoints for
accessing the Faasm deployment.

If you need to regenerate this, you can run:

```bash
inv knative.ini-file
```

## Uploading and invoking functions

Once you have configured your `faasm.ini` file, you can use the Faasm, CPP,
Python CLIs via the [`docker-compose-k8s.yml`](../docker-compose-k8s.yml) file
in this repo as normal, for example:

```bash
# Start the cpp container (make sure you've stopped all other Faasm containers)
docker-compose -f docker-compose-k8s.yml run cpp-cli /bin/bash

# Compile and upload a function
inv func demo hello
inv func.upload demo hello --host=<upload_host>

# Invoke the function
inv func.invoke demo hello --host=<invoke_host> --port=<invoke_port>
```

# Troubleshooting

First off you need to check that everything else is running. List all pods in
all namespaces and check for any whose status is not `Running`:

```bash
kubectl get pods --all-namespaces
```

Provided everything looks normal, or you find a container with something wrong,
you can look at logs. E.g. for a Faasm worker:

```bash
# Find the faasm-worker-xxx pod
kubectl -n faasm get pods

# Tail the logs for a specific pod
kubectl -n faasm logs -f faasm-worker-xxx user-container

# Tail the logs for all containers in the deployment
# You only need to specify the max-log-requests if you have more than 5
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
- Create a Kubernetes cluster **without Istio enabled**.
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
- Continue with Faasm installation as described above.

## MicroK8s

Install according to the [official docs](https://microk8s.io/).

Then enable RBAC, DNS and load balancer plugins with:

```bash
microk8s.enable dns rbac metallb:192.168.1.240/24
```

Note that you may have to set up different permissions to run without `sudo`
(although this is not necessarily required).

Make sure you have set up `kubectl` as per the
[docs](https://microk8s.io/docs/working-with-kubectl).

You can then continue with Faasm installation as described above.

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
