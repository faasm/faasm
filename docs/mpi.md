# Faasm MPI support

MPI translates quite nicely to a serverless context, but compiling any existing MPI implementation
to wasm proved tricky (see below). As a result, Faasm includes a minimal MPI implementation in `lib-faasmp`.  

## MPI Functions

A demo MPI function is found at `func/omp/hellompi.cpp`. You can compile, upload and invoke it with the following:

```
inv compile omp hellompi
inv upload omp hellompi
inv invoke omp hellompi
```

## Extending the Faasm MPI implementation

The MPI interface declarations live in `lib-faasmp` and the definitions in `src/wasm/syscalls_mpi.cpp`.

Anything missing needs to be added in both places, along with an entry in `lib-faasmp/faasmp.imports`. 

## Installing/ running locally

To install the latest Open MPI locally you can use the following Ansible playbook:
 
```
cd ansible
ansible-playbook mpi.yml --ask-become-pass
```

This installs it to `/usr/local/faasm/open-mpi`.

Once you've built a native executable linked against this, you can then use `mpirun` on the binary e.g.

```
mpirun -n 2 cmake-build-debug/bin/hellompi 
```

## Building Open MPI to WebAssembly

It should be possible to compile some of Open MPI to WebAssembly, but I couldn't quite get it to work.
To get the source you can add Open MPI as a submodule of this project at `third-party/ompi` and make 
sure you pin it to a specific release (e.g. `cd third-party/ompi && git checkout v4.0.2`).

Building Open MPI from source is described [here](https://github.com/Shillaker/ompi/blob/master/HACKING).
You need to make sure you have relatively up to date versions of `m4`, `autoconf`, `automake` and `libtool`
as listed [here](https://www.open-mpi.org/source/building.php) (on Ubuntu 18.04 mine were ok by default).

You can then run the `autogen.pl` script to generate the `configure` file and end up with a standard 
configure/ make process. You can build this as we build other configure/ make projects, by setting 
the relevant environment variables to invoke the Faasm toolchain (`CC`, `CXX`, `CFLAGS` etc.).

