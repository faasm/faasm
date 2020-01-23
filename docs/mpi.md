# MPI support

MPI could work really nicely in a serverless context, but compiling Open MPI to wasm 
proved tricky. With enough effort it _should_ be possible to get something simple compiling
(e.g. just being able to send integers between modules).  

## Building Open MPI to WebAssembly

To get the source you can add Open MPI as a submodule of this project at `third-party/ompi` and make 
sure you pin it to a specific release (e.g. `cd third-party/ompi && git checkout v4.0.2`.

Building from source is described [here](https://github.com/Shillaker/ompi/blob/master/HACKING).
You need to make sure you have relatively up to date versions of `m4`, `autoconf`, `automake` and `libtool`
as listed [here](https://www.open-mpi.org/source/building.php) (on Ubuntu 18.04 mine were ok by default).

You can then run the `autogen.pl` script to generate the `configure` file and end up with a standard 
configure/ make process. You can build this as we build other configure/ make projects, by setting 
the relevant environment variables to invoke the Faasm toolchain (`CC`, `CXX`, `CFLAGS` etc.).

Unfortunately I couldn't get this to work even after a few hours of tweaking. It fails on the configure 
phase, complaining either about the wasm architecture or not recognising generated object files (as they're
wasm). There are lots of options that can be passed to configure to simplify the build, and I'm sure it 
could be reduced to a really small subset of functionality and be made to work.

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
