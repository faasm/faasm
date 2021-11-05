<div align="center">
<img src="https://raw.githubusercontent.com/faasm/faasm/master/faasm_logo.png"></img>
</div>

# Faasm [![Tests](https://github.com/faasm/faasm/workflows/Tests/badge.svg?branch=master)](https://github.com/faasm/faasm/actions)  [![License](https://img.shields.io/github/license/faasm/faasm.svg)](https://github.com/faasm/faasm/blob/master/LICENSE.md)  [![Release](https://img.shields.io/github/release/faasm/faasm.svg)](https://github.com/faasm/faasm/releases/)  [![Contributors](https://img.shields.io/github/contributors/faasm/faasm.svg)](https://github.com/faasm/faasm/graphs/contributors/)

Faasm is a high-performance stateful serverless runtime.

Faasm provides multi-tenant isolation, yet allows functions to share regions of
memory. These shared memory regions give low-latency concurrent access to data,
and are synchronised globally to support large-scale parallelism.

Faasm combines software fault isolation from WebAssembly with standard Linux
tooling, to provide security and resource isolation at low cost. Faasm runs
functions side-by-side as threads of a single runtime process, with low
overheads and fast boot times.

Faasm is built on [Faabric](http://github.com/faasm/faabric) which provides
the distributed messaging and state layer.

The underlying WebAssembly execution and code generation is built using
[WAVM](https://github.com/WAVM/WAVM).
Support for running functions inside SGX enclaves is achieved using
[WAMR](https://github.com/bytecodealliance/wasm-micro-runtime).

Faasm defines a custom [host interface](docs/host_interface.md) which extends
[WASI](https://wasi.dev/) to include function inputs and outputs, chaining
functions, managing state, accessing the distributed filesystem, dynamic
linking, pthreads, OpenMP and MPI.

Our paper from Usenix ATC '20 on Faasm can be found
[here](https://www.usenix.org/conference/atc20/presentation/shillaker).

# Quick start

Update submodules:

```bash
git submodule update
```

Start a Faasm cluster locally using `docker-compose`:

```bash
docker-compose up -d --scale worker=2 nginx
```

To compile, upload and invoke a C++ function using this local cluster you can
use the [faasm/cpp](https://github.com/faasm/cpp) container:

```bash
docker-compose run cpp /bin/bash

# Compile the demo function
inv func demo hello

# Upload the demo "hello" function
inv func.upload demo hello

# Invoke the function
inv func.invoke --host=nginx demo hello
```

## More information

More detail on some key features and implementations can be found below:

- [Usage and set-up](docs/setup.md) - using the CLI and other features.
- [C/C++ functions](https://github.com/faasm/cpp) - writing and deploying Faasm functions in C/C++.
- [Python functions](https://github.com/faasm/python) - isolating and executing functions in Python.
- [Distributed state](docs/state.md) - sharing state between functions.
- [Faasm host interface](docs/host_interface.md) - the serverless-specific interface between functions and the underlying host.
- [Kubernetes and Knative integration](docs/kubernetes.md)- deploying Faasm as part of a full serverless platform.
- [Bare metal/ VM deployment](docs/bare_metal.md) - deploying Faasm on bare metal or VMs as a stand-alone system.
- [API](docs/api.md) - invoking and managing functions and state through Faasm's HTTP API.
- [MPI](docs/mpi.md) and [OpenMP](docs/openmp.md) - executing existing MPI and OpenMP applications in Faasm.
- [Developing Faasm](docs/development.md) - developing and modifying Faasm.
- [Releases](docs/releases.md) - instructions for releasing new versions and building container tags.
- [Faasm.js](https://github.com/faasm/faasmjs) - executing Faasm functions in the browser and on the server.
- [Threading](docs/threads.md) - executing multi-threaded applications.
- [Proto-Faaslets](docs/proto_faaslets.md) - snapshot-and-restore to reduce cold starts.
- [WAMR support](docs/wamr.md) - support for the [wasm-micro-runtime](https://github.com/bytecodealliance/wasm-micro-runtime) (WIP).
- [SGX](docs/sgx.md) - information on executing functions with [SGX](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html) (WIP).

## Experiments and benchmarks

- [Tensorflow Lite](https://github.com/faasm/faasm-experiments/blob/master/docs/tensorflow.md) - performing inference in Faasm with [Tensorflow Lite](https://www.tensorflow.org/lite/)
- [Polybench](https://github.com/faasm/faasm-experiments/blob/master/docs/polybench.md) - benchmarking with [Polybench/C](http://web.cse.ohio-state.edu/~pouchet.2/software/polybench/)
- [ParRes Kernels](https://github.com/faasm/faasm-experiments/blob/master/docs/prk.md) - benchmarking with the [ParRes Kernels](https://github.com/ParRes/Kernels)
- [Python performance](https://github.com/faasm/faasm-experiments/blob/master/docs/python.md) - executing the [Python performance benchmarks](https://github.com/python/pyperformance)

# Acknowledgements

This project has received funding from the European Union's Horizon 2020
research and innovation programme under grant agreement No 825184 (CloudButton),
the UK Engineering and Physical Sciences Research Council (EPSRC) award 1973141,
and a gift from Intel Corporation under the TFaaS project.
