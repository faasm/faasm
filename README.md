# Faasm

Faasm is a serverless system focused on performance and security. By trusting users' code we can reduce
isolation, hence achieving good performance while simultaneously enabling features not possible in a 
strongly isolated environment.

The project is still a work in progress with many aspects yet to be developed.

# Isolation

## Memory

By accepting only functions that can be compiled to WebAssembly, we can rule out a large class of security 
issues related to memory. The specifics of the WebAssembly memory model and its security implications
can be found in [the WebAssembly docs](https://webassembly.org/docs/security/).

## CPU

Linux cgroups support process-level and thread-level CPU isolation and accounting and can be enlisted to 
establish isolation between users' functions in Faasm. 

Thread-level support is only available in `cgroupv2` which is currently incompatible with Docker. How this
interaction will work is TBC.

## Network

TBC

## Filesystem

A filesystem doesn't make much sense in a serverless environment given that functions may run on any machine 
in the system. As a result the role of the filesystem is greatly diminished.

## Other I/O

I/O other than that concerned with the filesystem and network is not supported in a serverless environment thus can be excluded. 

# Usage

## Functions

Each function is associated with a user and has a function name. It will have two URLs:

- Synchronous - `<faasm_host>/f/<user>/<function>/`
- Asynchronous - `<faasm_host>/fa/<user>/<function>/`

By `POST`ing to these URLs we can invoke the function. POSTed data forms the input data for the function call.

For example, with the faasm endpoint at `localhost:8080`, the `echo` function owned by `simon` can be run with:

```
curl -X POST http://localhost:8080/f/simon/echo -d "hello faasm"
```

This function just returns its input so should give a response containing our input data (i.e. `hello faasm`).
The code can be found in `func/function_echo.c`.

## Writing Functions

The function API passes a number of pointers to functions to allocate memory regions. A convenience header is
provided in at `include/faasm/faasm.h`. If compiling with Emscripten, your functions will look something like this:

```
#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    // Do something

    return 0;
}
```

### `faasm.h`

`faasm.h` contains some useful wrappers to make it easier to interact with the Faasm system.

The `FaasmMemory` struct represents the memory available to Faasm functions. It has the following fields:

- `input` - this is an array containing the input data to the function
- `output` - this is where the function can write its output data
- `chainFunctions`, `chainInputs`, `chainCount` - these are internal values used to handle function "chaining" (see below)

### Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user).

To do this, `chainFunction()` in `faasm.h` can be called. For my function to invoke the function `foo`,
(also owned by me), it can do the following:

```
#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    uint8_t funcData[] = {1, 2, 3, 4};
    int dataLength = 4;
    char* funcName = "foo";

    chainFunction(memory, funcName, funcData, dataLength);

    return 0;
}
```

`chainFunction` can be called multiple times in one function. Once the original function has completed, these
calls will go back through the main scheduler and be executed.

## Uploading Functions

To upload a function you can use `curl` to send a PUT request to the synchronous URL for the given function.
For example:

- I have a Faasm endpoint running at `localhost:8080`
- I've compiled my WebAssembly function file to `/tmp/do_something.wasm`
- I want to upload this function to user `simon` and function name `cool_func`

I can execute:

```
curl http://localhost:8080/f/simon/cool_func/ -X PUT -T /tmp/do_something.wasm
```

# Kubernetes

Faasm can be deployed to Kubernetes to create a distributed set-up. The configuration files are found in the `k8s`
directory. This has the following components:

- A load balancer handling incoming calls
- A single `edge` pod which parses the calls and input data
- A `redis` pod which holds the function calls in a queue
- Multiple `worker` pods which pull calls off the queue, execute them (and any chained calls), then put the results into another Redis queue.

## Using the Kubernetes deployment

Currently we have a cluster running on several Imperial machines with the master at `maru17.doc.res.ic.ac.uk`. To use
this deployment you can SSH onto this machine, then get the URL for the Faasm service by running:

```
kubectl get service edge --namespace=faasm
```

You can then hit one of the dummy functions with:

```
curl -X POST http://<endpoint_ip>:8080/f/simon/echo/ -d "hello world"
```

## Deploying to Kubernetes

Deployment to Kubernetes is handled via the make target:

```
make setup-k8
```

## Function storage

Clearly all worker pods need access to the WASM function files. For now these are held on an NFS share hosted on the
master Kubernetes node. This could be replaced by an object store in future.

# Installation, Configuration and Development

Below are instructions for building, testing and developing.

## Local Development

The local development process is a bit rough around the edges at the moment but can be improved in future.

### CGroup V2

To enable `cgroupv2` you need to do the following:

- Add `cgroup_no_v1=all systemd.unified_cgroup_hierarchy=1` to the line starting `GRUB_CMDLINE_LINUX_DEFAULT` in `/etc/default/grub`(separated by spaces)
- Run `sudo update-grub`
- Restart the machine 

To check it's worked, the file `/sys/fs/cgroup/cgroup.controllers` should exist and contain something like:

```
cpu io memory pids rdma
```

Note that currently **Docker does not work with cgroupv2** so this is only applicable to machines running Faasm outside of Docker.

### Submodules

Faasm relies on WAVM which needs to be updated via a Git submodule (once after original checkout of this repo).

```
# Update submodules
git submodule update --remote --init
```

### Libraries

Faasm has various library dependencies that can be installed with Ansible using the relevant `make` target:

```
make setup-libs
```

### Protobuf

Faasm depends on protobuf which can be a bit of a hassle to install. First of all you can try the make target:

```
make setup-protobuf
```

If there are any issue you need to remove every trace of protobuf on your system before reinstalling.

You can look in the following folders and remove any reference to `libprotobuf` or `protobuf`:

- `/usr/lib/x86_64-linux-gnu/`
- `/usr/lib/`
- `/usr/include/google`

Avoid trying to do this with `apt` as it can accidentally delete a whole load of other stuff.

### Clang

So far Faasm has only been built with clang (although there's no reason not to use something else).
There's a make target for installing clang at:

```
make setup-clang
```

### Redis

At the moment Faasm uses Redis for messaging between the edge and worker nodes. For running locally you'll need to
install it on your machine:

```
sudo apt install redis-server redis-tools
```

### Building with CMake

Faasm also requires an up-to-date version of CMake. Once you have this you can run the following from the root of
your project:

```
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target all
```

I usually develop through CLion which makes it a lot easier.

### Running Locally

Once things are built, you can run a simple local set-up with:

```
# Set up function storage path (see below)
export FUNC_ROOT=<path to the root of this project>

# Edge node listening on localhost:8080
./build/bin/edge

# Worker
./build/bin/worker
```

The `FUNC_ROOT` is where Faasm will look for function files. For a function called `dummy` owned by user
`simon` the `.wasm` file should be found at `$FUNC_ROOT/wasm/simon/dummy/function.wasm`. When you upload a function
this is where it will get placed too.

### Dummy functions

Currently there are some dummy functions held in the `func` directory. Their compiled WASM is also stored in the
`wasm` directory.

### Docker images

There are a few Docker images used to make build times quicker:

- `shillaker/cpp-base` - from [this repo](https://github.com/Shillaker/cpp-base), just a base image that includes clang and protobuf (as they're a pain to install)
- `shillaker/faasm-base` - image extending `cpp-base` with the extra faasm-specific libraries installed
- `shillaker/faasm-core` - image extending `faasm-base` with the faasm code in place

If you're just changing code, all you need to rebuild is `faasm-core`. If you change libraries you'll need to rebuild
`faasm-base` too.

### Tests

The tests can be found in the `test` directory and executed by running:

```
./build/bin/tests
```

They require a local Redis instance to pass and cover most of the codebase.
