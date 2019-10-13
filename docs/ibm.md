# IBM Cloud

You can see all your current resources [here](https://cloud.ibm.com/resources).

## API Key

In the browser create an IAM API key and write it to `~/faasm/faasm.ini`

```
[IBM]
email = <YOUR ACCOUNT EMAIL>
api_key = <API KEY HERE>
```

## IBM CLI

Install docs [are here](https://cloud.ibm.com/docs/cli?topic=cloud-cli-getting-started#overview) but you need to run:

```
# Install and check
curl -sL https://ibm.biz/idt-installer | bash
ibmcloud dev help

# Install plugins
ibmcloud plugin install container-service -f
ibmcloud plugin install container-registry -f
ibmcloud plugin install cloud-object-storage -f
ibmcloud plugin install cloud-functions -f
ibmcloud plugin install kubernetes-service -f
```

Once done and API key is in your config, run 

```
inv ibm-login
```

Choose `eu-gb` if it asks.

## Cloud Object Store

Set up a cloud storage service called `faasm-storage`. Then get the CRN and ID associated with this service you can run:

```
ibmcloud resource service-instance faasm-storage --id
```

NOTE: this returns something of the form:

```
<CRN> <ID>

# EXAMPLE:
crn:v1:bluemix:public:cloud-object-storage:global:a/453frr8338c0444433a8d97ddee679cab7:c44465da-a4d9-4524-98s8-afe13b00a45f:: fd336265da-a5c9-45a4-9868-abgg1b00a45f
```

You must split this into the two bits in `~/faasm/faasm.ini` including quotes:

```
[IBM]
cos_crn = "<YOUR COS CRN>"
cos_id = "<YOUR COS ID>"
```

You then need to configure the CLI:

```
# Copy your CRN at the prompt
ibmcloud cos config crn

# Couple of other tweaks
inv ibm-set-up-cos
```

To create the required buckets:

```
inv ibm-create-buckets
```

## KNative

There's a tutorial [here](https://github.com/IBM/knative101/blob/master/workshop/exercise-0/README.md) but the steps are as follows.

First install the client-side tooling:

```
cd ansible
ansible-playbook kube_client.yml --ask-become-pass
```

Then you need to set up a cluster on IBM Cloud [here](https://cloud.ibm.com/kubernetes/catalog/cluster/create). Add as much resource as you can afford.

Once provisioned you'll get a cluster ID (e.g. `dme8c74l0kfe7el1lbe2`). Add this to `~/faasm/faasm.ini`:

```
[IBM]
...
k8s_cluster_id = <CLUSTER ID>
```

You then need to wait for it to start up:

```
watch ibmcloud ks clusters
```

Once it's ready you can run:

```
inv ibm-k8s-add-knative
```

This will request that knative be installed. You need to wait until everything is ready before proceeding:

```
watch kubectl get pods --all-namespaces
```

Once everything is `Running`, you can run the following:

```
inv ibm-k8s-config
```

This will give you a `KUBECONFIG` variable to export in your sessions.

Finally you need to get the ingress subdomain from your cluster's information in the browser (e.g. `faasm.eu-gb.containers.appdomain.cloud`). Add this to `~/faasm/faasm.ini`:

```
[IBM]
...
k8s_subdomain = <SUBDOMAIN>
```

### Deploying Faasm

Provided you have the right `KUBECONFIG` exported you should be able to deploy Faasm according to the `kubernetes.md` doc.

# ---------------------------------------------------
# Legacy
# ---------------------------------------------------

## Namespace

Create a cloud functions namespace [here](https://cloud.ibm.com/functions/namespace-settings). Call it `faasm-namespace`.

Get the GUID and add to `~/faasm/faasm.ini`:

```
[IBM]
namespace_id = <NS_ID_HERE>
```

## Redis Virtual Machine

Create a new Ubuntu Virtual Server in the IBM Cloud [here](https://cloud.ibm.com/gen1/infrastructure/provision/vs). Unfortunately the network on the shared servers is incredibly slow even with a 1Gbps NIC.

- Dedicated instance, auto-assign
- Set the hostname/domain as `redis.faasm.lsds`
- Select a London DC
- 2 vCPU 4GB RAM
- 1Gbps NIC
- Ubuntu 18.04 minimal (make sure the box is actually ticked)
- Add your SSH key
- Set `allow_all` and `allow_outbound` both the public and private security groups
- Double check everything in the sidebar

Note that there are two IPs, public and private. We need to use the public for our provisioning, but the application needs to use the private.

Once the VM is up, you need to add a file at `ansible/inventory/ibm_cloud.yml` with:

```
[redis]
<PUBLIC IP>   ansible_ssh_user=root
```

Then edit `~/faasm/faasm.ini` with:

```
[IBM]
...
redis_host_private = <PRIVATE IP>
redis_host_public = <PUBLIC IP>
```

Check you can SSH and update the software:

```
inv ibm-ssh-redis

# On the host
sudo apt-get update && sudo apt-get upgrade -y
```

Then you can set up Redis with:

```
inv ibm-set-up-redis
```

## Links

- [KNative tutorial](https://github.com/IBM/knative101/blob/master/workshop/exercise-0/README.md)
- Python Cloud Object Store API [here](https://cloud.ibm.com/docs/services/cloud-object-storage/libraries?topic=cloud-object-storage-python)
- Virtual server CLI docs [here](https://cloud.ibm.com/docs/cli?topic=cloud-cli-cli-virtual-servers)
- CLI docs [here](https://cloud.ibm.com/functions/learn/cli)
- Docker set-up instructions [here](https://cloud.ibm.com/docs/openwhisk?topic=cloud-functions-prep#prep_docker)
- Free academic licenses from [here](https://ibm.biz/academic)
- [Cloud storage curl API](https://cloud.ibm.com/docs/services/cloud-object-storage/cli?topic=cloud-object-storage-curl)
- [IBM Docker Python handler](https://github.com/apache/openwhisk-runtime-docker/blob/master/core/actionProxy/actionproxy.py).
- [Docs on COS CLI](https://cloud.ibm.com/docs/services/cloud-object-storage/cli?topic=cloud-object-storage-ic-use-the-ibm-cli).
- Docs on cloud functions CLI [here](https://cloud.ibm.com/docs/openwhisk?topic=cloud-functions-cli-plugin-functions-cli)
- [IBM PyWren repo](https://github.com/pywren/pywren-ibm-cloud)
- [IBM PyWren config](https://github.com/pywren/pywren-ibm-cloud/tree/master/config)

