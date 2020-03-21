# Faasm on bare metal/ VMs

Faasm can run on any standard bare metal machine or VM with Ubuntu 18.04. 

## Set-up

The set-up is automated with Ansible and run through a set of Faasm CLI functions. 

### Inventory file

First you need to create a file at `ansible/inventory/bare_metal.yml` which groups the 
machines into their roles in Faasm.

To avoid ambiguity on machines with multiple network interfaces, you must explicitly 
specify the hostname or IP to be used by Faasm with `faasm_hostname=...`. If the 
machines only have a single network interface, this is just the machine's normal 
hostname or IP.

The file will look something like:

```ini
[all]
host1  faasm_hostname=<hostname/ IP>
host2  faasm_hostname=<hostname/ IP>
host3  faasm_hostname=<hostname/ IP>
host4  faasm_hostname=<hostname/ IP>

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
