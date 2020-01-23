# Faasm MPI support

A demo MPI function can be found at `funcs/omp/hellompi.cpp`.

## Compiling functions

To compile and run the demo MPI function:

```
inv compile omp hellompi
inv upload omp hellompi
inv invoke omp hellompi
```

## Compiling native functions

To compile MPI functions locally you need to install OpenMPI.

To do this, you must first make sure you clear out any existing MPI 
installations, e.g.

```
sudo apt remove --purge libmpi-dev openmpi-bin openmpi-common
```

Then you can use the Ansible playbook in this repo:

```
cd ansible
ansible-playbook mpi.yml --ask-become-pass
```

## Running native functions

Once you've built the native executable, you need to use `mpirun` on the binary 
you built, e.g.

```
mpirun -n 2 cmake-build-debug/bin/hellompi 
```

