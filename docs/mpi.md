# Faasm MPI support

Faasm provides a custom MPI implementation to execute existing unmodified MPI 
applications in a serverless context. 

The Faasm MPI implementation is minimal but covers the majority of commonplace MPI
functionality (point-to-point, collective, one-sided, custom datatypes).   

## MPI Functions

A number of MPI functions can be found at `func/mpi`. You can compile, upload and invoke 
`hellompi` with the following:

```
inv compile mpi hellompi
inv upload mpi hellompi
inv invoke mpi hellompi
```

## Running code locally

To install the latest Open MPI locally you can use the following Ansible playbook:
 
```
cd ansible
ansible-playbook openmpi.yml --ask-become-pass
```

This installs it to `/usr/local/faasm/openmpi`.

Once you've built a native executable linked against this, you can then use `mpirun` on the binary e.g.

```
/usr/local/faasm/openmpi/bin/mpirun -n 2 <your native mpi func> 
```

## ParRes Kernels

We can benchmark Faasm's MPI implementation using the [ParRes Kernels](https://github.com/ParRes/Kernels)
modified slightly in the fork found in `third-party/ParResKernels`.

To compile and upload you can run the following:

```bash
inv libs.prk
inv upload.user prk
```

This uploads a number of the kernels written for MPI, e.g. `nstream`. These can be invoked using:

```bash
inv prk.invoke nstream
```

# Benchmarks

We benchmark Faasm against OpenMPI by running both on the same set of bare metal
machines or VMs. The steps to do this are as follows.

## 0. Check SSH

Note that all the machines must be able to SSH to each other without any 
prompt or passwords. 

## 1. Set up the VMs

Follow the normal Faasm [bare metal setup](bare_metal.md).

## 2. Install OpenMPI

Run the Ansible playbook:

```bash
cd ansible
ansible-playbook -i inventory/mpi.yml mpi_benchmark.yml
```

## 3. Run native code

The set-up process will create a hostfile at `~/mpi_hostfile` according to your inventory.
This is automatically used by the underlying scripts to run the benchmarks.

To check the native code works, you can run:

```bash
inv prk.invoke nstream --native
```

## 4. Run wasm code

Faasm functions using MPI can be run as with any others. To check the basic WASM MPI set-up, run:

```bash
inv upload.user mpi
inv invoke mpi mpi_checks
```

To check this works you can run:

```bash
inv upload.user prk
inv prk.invoke nstream
``` 

## Troubleshooting Native MPI

Remember that all machines must be able to SSH onto each other with no prompts (passwords or confirmations).

Machines with multiple network interfaces may sometimes need to specify which interfaces to use, e.g.:

```bash
mpirun ... -mca btl_tcp_if_include eth1 ...
```

You can also specify CIDR address ranges:

```bash
mpirun ... -mca btl_tcp_if_include 192.168.0.0/16 ...
```

## Extending the Faasm MPI implementation

The MPI interface declarations live in `libs/faasmpi` and the definitions in `src/wasm/mpi.cpp`.

Any new functions need to be included in `libs/faasmpi/faasmpi.imports`. 
