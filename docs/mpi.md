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

## Extending the Faasm MPI implementation

The MPI interface declarations live in `libs/faasmpi` and the definitions in `src/wasm/mpi.cpp`.

Any new functions need to be included in `libs/faasmpi/faasmpi.imports`. 
