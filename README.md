<div align="center">
<img src="https://raw.githubusercontent.com/lsds/Faasm/master/faasm_logo.png"></img>
</div>

# Faasm [![Tests](https://github.com/lsds/Faasm/workflows/Tests/badge.svg?branch=master)](https://github.com/lsds/Faasm/actions)  [![License](https://img.shields.io/github/license/lsds/Faasm.svg)](https://github.com/lsds/Faasm/blob/master/LICENSE.md)  [![Release](https://img.shields.io/github/release/lsds/Faasm.svg)](https://github.com/lsds/Faasm/releases/)  [![Contributors](https://img.shields.io/github/contributors/lsds/Faasm.svg)](https://github.com/lsds/Faasm/graphs/contributors/)

Faasm is a high-performance stateful serverless runtime. 

Faasm provides multi-tenant isolation, but also lets functions share regions of memory. 
These shared memory regions give low-latency concurrent access to data, and are synchronised 
globally to support large-scale parallelism.

Faasm combines software fault isolation from WebAssembly with standard Linux tools to isolate functions
in Faaslets, which provide security and resource isolation at low cost. Faaslets run side-by-side as threads 
of a single runtime process, with low overheads and fast boot times. The underlying WebAssembly
execution and code generation is handled by [WAVM](https://github.com/WAVM/WAVM). 

Faasm defines a custom [host interface](docs/host_interface.md) which lets functions perform 
serverless-specific tasks (e.g. invoking other functions and managing state), as well as interacting 
with the underlying host (e.g. using the filesystem and networking). The Faasm host interface achieves
the same goal as [WASI](https://wasi.dev/), but in a serverless-specific context.

## Features

More detail on some key features and implementations can be found below: 

- [C/C++ functions](docs/cpp.md) - writing and deploying Faasm functions in C/C++.
- [Python functions](docs/python.md) - isolating and executing functions in Python.
- [Faasm host interface](docs/host_interface.md) - the serverless-specific interface between functions and the underlying host.
- [Distributed state](docs/state.md) and [scheduling](docs/scheduling.md) - sharing state and function executions between distributed Faasm instances.
- [Kubernetes and Knative integration](docs/kubernetes.md)- deploying Faasm as part of a full serverless platform.
- [Tensorflow Lite](docs/tensorflow.md) - performing inference in Faasm functions with TFLite.  
- [MPI and OpenMP](docs/openmp.md) - executing existing MPI and OpenMP applications in Faasm.
- [Browser offloading with Faasm.js](https://github.com/Shillaker/faasmjs) - executing Faasm functions in the browser and on the server. 
- [API](docs/api.md) - invoking and managing functions and state through Faasm's HTTP API.
- [Snapshot and restore with Proto-Faaslets](docs/proto_faaslets.md) - cutting down cold starts with execute-once initialisation. 

# Quick start

You can start a Faasm cluster locally using the `docker-compose.yml` file in the root of the project:

```bash
docker-compose up --scale worker=3
```

This creates three Faasm instances as well as an upload endpoint for receiving functions and state, 
plus a Redis instance for communication and storage.

Note that the first time you run the local set-up it will generate some machine code specific to your host. This is
stored in the `machine-code` directory in the root of the project and reused on subsequent runs.

See the docs linked above for more info on writing and deploying functions.