# Usage

## Invoking Functions

Each function is associated with a user and has a function name. It will have two URLs:

- Synchronous - `<faasm_host>/f/<user>/<function>/`
- Asynchronous - `<faasm_host>/fa/<user>/<function>/`

By `POST`ing to these URLs we can invoke the function.

For example with the faasm endpoint at `localhost:8080`, to test the `echo` function owned by user `simon` we can run:

```
curl -X POST http://localhost:8080/f/simon/echo -d "hello faasm"
```

We should then see a response containing our input data (i.e. `hello faasm`).

## Writing Functions

The function API is a little verbose as it passes a number of pointers around to handle memory management.
A convenience header is provided in this repo at `include/faasm/faasm.h`. If compiling with Emscripten, your functions
will look something like this:

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

The `FaasmMemory` struct is a convenience wrapper to make memory management easier. It has the following properties:

- `input` - this is an array containing the input data to the function
- `output` - this is where the function can write its output data
- `chainFunctions`, `chainInputs`, `chainCount` - these are internal values used to handle function "chaining"

### Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user).

To do this, the `chainFunction` function in `faasm.h` can be called.

My function can invoke the function `foo`, passing some input data as follows:

```
#include "faasm.h"

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    chainFunction(memory, "foo", funcData, dataLength);

    return 0;
}
```

## Uploading Functions

To upload a function you can use `curl` to send a PUT request to the synchronous URL for the given function.
For example, if I:

- Have a faasm endpoint running at `localhost:8080`
- Have a compiled function file at `/tmp/do_something.wasm`
- Want to upload this function to user `simon` and function name `cool_func`

I would execute:

```
curl http://myhost:8080/f/simon/cool_func/ -X PUT -T /tmp/do_something.wasm
```

# Kubernetes

Faasm can be deployed to Kubernetes to create a distributed set-up. The configuration files are found in the `k8s`
directory.

## Deploying to Kubernetes

Deployment to Kubernetes is handled via the make target:

```
make setup-k8
```

# Installation, Configuration and Development

## Local Development

### Submodules

Faasm relies on WAVM which needs to be updated via a Git submodule

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

I've only used clang to build faasm so far. There's a make target for it:

```
make setup-clang
```

### Redis

At the moment Faasm uses Redis for messaging between the edge and worker nodes. For running locally you'll need to
install it:

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

I usually develop through CLion which does a lot of this stuff for you.

### Running Locally

Once things are built, you can run a simple local set-up by running:

```
export FUNC_ROOT=<your path to the root of this project>

# Edge node listening on 8080
./build/bin/edge

# Worker
./build/bin/worker
```

Note that the FUNC_ROOT is where Faasm will look for function files. For a function called `dummy` owned by user
`simon` the `.wasm` file will be at `$FUNC_ROOT/wasm/simon/dummy/function.wasm`. Likewise, when you upload a function
it will get written here.

### Dummy functions

Currently there are some dummy functions held in the `func` directory. Their compiled WASM is also stored in the
`wasm` directory.
