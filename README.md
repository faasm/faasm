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

### Tests

The tests can be found in the `test` directory and executed by running:

```
./build/bin/tests
```

They require a local Redis instance to pass and cover most of the codebase.