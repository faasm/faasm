Faasm Documentation
===================================

Faasm is a high-performance stateful serverless runtime.

Faasm provides multi-tenant isolation, yet allows functions to share regions of
memory. These shared memory regions give low-latency concurrent access to data,
and are synchronised globally to support large-scale parallelism.

Faasm combines software fault isolation from WebAssembly with standard Linux
tooling, to provide security and resource isolation at low cost. Faasm runs
functions side-by-side as threads of a single runtime process, with low
overheads and fast boot times.

Faasm uses `Faabric <https://github.com/faasm/faabric/>`_ as a distributed
messaging and state layer.

Faasm defines a custom :doc:`host interface </source/host_interface>` which
extends `WASI <https://wasi.dev/>`_ to include function inputs and outputs,
chaining functions, managing state, accessing the distributed filesystem,
dynamic linking, pthreads, OpenMP and MPI.

Our paper from Usenix ATC '20 on Faasm can be found `here
<https://www.usenix.org/conference/atc20/presentation/shillaker>`_.

Experiments and benchmarks:

- `Microbenchmarks (C++ and Python) <https://github.com/faasm/experiment-microbench/>`_
- `MPI <https://github.com/faasm/experiment-mpi>`_
- `OpenMP <https://github.com/faasm/experiment-openmp>`_

.. toctree::
   :maxdepth: 1
   :caption: Topics

   source/api
   source/bare_metal
   source/cpp
   source/debugging
   source/development
   source/host_interface
   source/kubernetes
   source/mpi
   source/networking
   source/profiling
   source/proto_faaslets
   source/python
   source/releases
   source/setup
   source/sgx
   source/state
   source/threads
   source/wamr

.. toctree::
   :maxdepth: 1
   :caption: Code

   apidoc/namespacelist
   apidoc/classlist
