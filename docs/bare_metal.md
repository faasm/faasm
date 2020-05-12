# Faasm on bare metal/ VMs

Faasm can run on any standard bare metal machine or VM with Ubuntu 18.04. 

## Set-up

The set-up is automated with Ansible and run through a set of Faasm CLI functions. 

### Inventory file

First you need to create a file at `ansible/inventory/bare_metal.yml` which groups the 
machines into their roles in Faasm.

To avoid ambiguity on machines with multiple network interfaces, you must explicitly 
specify the hostname or IP to be used by Faasm with `mpi_host=...`. If the 
machines only have a single network interface, this is just the machine's normal 
hostname or IP.

The file will look something like:

```ini
[all]
host1  mpi_host=<host1/ IP1>
host2  mpi_host=<host2/ IP2>
host3  mpi_host=<host3/ IP3>
host4  mpi_host=<host4/ IP4>

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
source workon.sh

inv upload demo hello
inv invoke demo hello --input="hello!"
```

## Troubleshooting

Faasm is run using [supervisor](https://github.com/Supervisor/supervisor) on each 
of the hosts. 

Logs can be found at:

- `/var/log/faasm_upload.log` - upload server logs (on the `upload` host)
- `/var/log/faasm_worker.log` - worker logs (on `worker` hosts)

## Google Cloud 

Faasm can be run in "bare metal" mode on a collection of standard GCP VMs. Once 
set up, you can follow the instructions above to install Faasm.

You can set up as many worker VMs as you want, but three is the recommended minimum.
An example script to do the set-up might look like the following (provided you've set 
up the [Cloud SDK](https://cloud.google.com/sdk)):

```bash
# Create a few instances
function create_vm() {
    gcloud compute instances create $1      \
        --image-project ubuntu-os-cloud     \
        --image-family ubuntu-1804-lts      \
        --machine-type e2-standard-4
}

create_vm faasm-worker-1
create_vm faasm-worker-2
create_vm faasm-worker-3
create_vm faasm-upload
    
# List instances
gcloud compute instances list
```

To SSH into the instances:

```bash
# Using the GCP CLI
gcloud compute ssh faasm-worker-1

# Or using standard SSH provided you've set up the SSH key through gcloud
ssh -i ~/.ssh/google_compute_engine <instance_ip>
```

Once things are set up and you can SSH, you can follow the instructions above.