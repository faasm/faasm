# Faasm on bare metal/ VMs

Faasm can run on any standard bare metal machine or VM with Ubuntu 18.04. 

## Set-up

The set-up is automated with Ansible and run through a set of Faasm CLI functions. 

### Inventory file

First you need to create a file at `ansible/inventory/bare_metal.yml` which groups the 
machines into their roles in Faasm.

To avoid ambiguity on machines with multiple network interfaces, you must explicitly 
specify the hostname or IP to be used by Faasm with `internal_host=...`. If the 
machines only have a single network interface, this is just the machine's normal 
hostname or IP. If there is more than one network interface, you can optionally specify 
the network interface.

The file will look something like:

```ini
[all]
host1  internal_host=<host1/ IP1> internal_iface=eth1
host2  internal_host=<host2/ IP2> internal_iface=eth1
host3  internal_host=<host3/ IP3> internal_iface=eth1
host4  internal_host=<host4/ IP4> internal_iface=eth1

[worker]
# Can be one or more hosts
host1
host2

[upload]
# Must be a single host
host3

[redis]
# Must be a single host
host3

[edge]
# Must be a single host
host4
```

### Set-up and install

You can then run the set-up on all the machines with the [Faasm CLI](setup.md):

```bash
# Initial machine set-up
inv bm.setup

# Faasm deploy
inv bm.deploy
```

### Checks

To check that Faasm is set up properly (and to run any subsequent Faasm commands),
you can SSH onto your `upload` host and run the following:

```bash
cd /usr/local/code/faasm
bash workon.sh

inv upload demo echo
inv invoke demo echo --input="hello!"
```

## Google Cloud 

Faasm can be run in "bare metal" mode on a collection of standard GCP VMs. Once 
set up, you can follow the instructions above to install Faasm.

You can set up Faasm on as VMs as you want, but [this script](../bin/gcp_minimal.sh)
sets up the minimum recommended configuration (assumes you've set up the 
[Cloud SDK](https://cloud.google.com/sdk)).

To SSH into the instances:

```bash
# Using the GCP CLI to get into worker 1
gcloud compute ssh faasm-worker-1

# Or using standard ssh with the default GCP SSH config
ssh -i ~/.ssh/google_compute_engine <instance_ip>
```

You can create the inventory file as described above, but make sure you specify the 
**internal IPs** for comms between the hosts, this will look something like:

```ini
[all]
<host1_public_ip>  internal_host=<host1_internal_ip>
<host2_public_ip>  internal_host=<host2_internal_ip>
...  
```

## Troubleshooting

Faasm is run using [supervisor](https://github.com/Supervisor/supervisor) on each 
of the hosts. 

Logs can be found at:

- `/var/log/faasm_upload.log` - upload server logs (on the `upload` host)
- `/var/log/faasm_worker.log` - worker logs (on `worker` hosts)
