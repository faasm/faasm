# Faasm

Faasm is a high-performance multi-tenant serverless runtime.

By using WebAssembly to run users' code, we can combine software fault isolation with 
standard OS tooling to provide strong security and resource isolation guarantees at low cost.

This lightweight isolation enables excellent performance and allows sharing of data between 
colocated functions through shared memory.

The underlying WebAssembly execution is handled by [WAVM](https://github.com/WAVM/WAVM), 
a server-side WebAssembly runtime which is well worth checking out.

More detail on the internals, development and deployment is held in the 
[wiki](https://github.com/lsds/faasm/wiki).

A preconfigured, Dockerised toolchain is provided for compiling C/C++ code to run in Faasm. 
This toolchaing is based on a generic [Dockerised WASM toolchain](https://github.com/Shillaker/wasm-toolchain)
with a customised libc to implement the required syscalls.

# Architecture

To allow integration with a range of serverless and data processing systems, Faasm workers have a pluggable input, 
output and state handling. These fall into the following categories:  

## Function calls, inputs and outputs

Faasm workers receive function calls as protobuf objects which are then deserialised to get the function name
and method of handling input/ output.

The following options for receiving messages are available:

- Redis "queues" - by specifying the `redis` mode and an associated key to listen for incoming invocations
- stdin pipe - by specifying the `pipe` mode
- sockets - by specifying the `socket` mode

Input and output can be handled as follows:

- Redis - when in `redis` mode, the inputs and outputs will be read from and written to the same Redis instance
- S3 - when in `s3` mode, inputs and outputs will be read from and written to the given S3 keys

## Function storage

To execute a function it must be available to the worker via one of the following sources:

- Local filesystem - at a predefined location (which can be mapped to NFS in a distributed environment)
- S3 - by giving an S3 bucket name

## State handling

Faasm workers support both mutable and immutable state accessed in either a synchronous or asynchronous manner.
This can be handled in the following ways:

- Redis - for both mutable and immutable state
- S3 - for immutable state only

# Serverless demo

To demonstrate a Faasm worker in action, the `docker-compose.yml` file in the root of the project will 
set up a simple serverless system using Redis to handle both function calls and state, and the local filesystem
to store functions. 

You can start it by running:

```
docker-compose up -d
```

## Compiling a simple function

The `hello.cpp` file in the root of this directory shows a basic faasm function; you can
open it and take a look at the API and format.

To compile this to WebAssembly you can run the following from the project root:

```
# Start the wasm toolchain container
docker run -v $(pwd):/work -w /work -it faasm/toolchain

# From inside the container, compile the hello.c file
/toolchain/bin/clang++ --target=wasm32-unknown-unknown-wasm --sysroot=/toolchain/sysroot -o hello.wasm -I include/faasm -lfaasm hello.cpp

# Drop out of the container
exit

# Upload the function
curl -X PUT http://localhost:8002/f/dummy/hello -T hello.wasm

# Invoke the function
curl -X POST http://localhost:8001/f/dummy/hello
```

You should then see the response `Hello faasm!`.

# Usage

## Functions

Each function is associated with a user and has a function name. It will have two URLs:

- Synchronous - `<faasm_host>/f/<user>/<function>/`
- Asynchronous - `<faasm_host>/fa/<user>/<function>/`

By `POST`ing to these URLs we can invoke the function. POSTed data forms the input data 
for the function call.

For example, with the faasm endpoint at `localhost:8001`, the `echo` function owned by 
`demo` can be run with:

```
curl -X POST http://localhost:8001/f/demo/echo -d "hello faasm"
```

The code for this function can be found in `func/demo/echo.cpp`.

## Writing Functions

Functions consist of an `exec` entrypoint which takes a pointer to a `FaasmMemory` object. 
This object is the interface for interacting with the Faasm runtime.

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Do something

        return 0;
    }
}
```

Some example functions can be found in the `func/demo` directory.

### `FaasmMemory`

The `FaasmMemory` class allows Faasm functions to interact with the system. 
It has the following methods:

- `getInput()` - allows functions to retrieve their input data
- `setOutput()` - this allows functions to return output data to the caller
- `chainFunction()` - this allows one function to invoke others
- `readState()` and `writeState()` - allows functions to read/ write key/value state
- `readStateOffset()` and `writeStateOffset()` - allows functions to read/ write at specific points in existing state (e.g. updating a subsection of an array)

## Chaining

"Chaining" is when one function makes a call to another function (which must be 
owned by the same user).

To do this, the `chainFunction()` method on the `FaasmMemory` instance can be called. 
For my function to invoke the function `foo`, (also owned by me), it can do the following:

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        uint8_t funcData[] = {1, 2, 3, 4};
        int dataLength = 4;

        memory->chainFunction("foo", funcData, dataLength);

        return 0;
    }
}
```

`chainFunction()` can be called multiple times in one function. Once the original 
function has completed, these calls will go back through the main scheduler and be executed.

## State

All of a users' functions have access to shared state. This state is implemented as a 
simple key-value store and accessed by the functions `readState` and `writeState` on the 
`FaasmMemory` object. Values read and written to this state must be byte arrays.

A function accessing state will look something like:

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "my_state_key";

        long stateSize = 123;

        // Read the state into a buffer
        uint8_t *myState = new uint8_t[stateSize];
        memory->readState(key, newState, stateSize);

        // Do something useful, modify state

        // Write the updated state
        memory->writeState(key, myState, stateSize);

        return 0;
    }
}
```

### Offset state

`FaasmMemory` also exposes the `readStateOffset` and `writeStateOffset` functions, 
which allow reading and writing subsections of byte arrays stored against keys in the 
key-value store.

For example, if I have an array of 100 bytes stored in memory, I can read bytes 15-20, 
then update them separately.

This can be useful for implementing distributed iterative algorithms.

### State example

An example implementing a simple counter can be found in `func/demo/increment.cpp`. It 
can be called with:

```
curl -X POST http://localhost:8001/f/demo/increment/
```

On successive calls this will add 1 to a value stored in shared state, returning something 
like:

```
Counter: 001
Counter: 002
Counter: 003
...
```

### Uploading state

If you need to prepopulate state for your functions you can use the state upload endpoint. 
This can be called with:

```
# Format /s/<username>/<key>
curl http://localhost:8002/s/user123/my_key -X PUT -d "your data"
```

This state can then be accessed in your functions using the specified key. For larger state, 
you can also upload a file:

```
curl http://localhost:8002/s/user123/my_key -X PUT -T /tmp/my_state_file
```

Where `/tmp/my_state_file` contains binary data you wish to be inserted at your specified key.

## Uploading Functions

To upload a function you can use `curl` to send a PUT request to the synchronous URL for the 
given function. For example:

- I have a Faasm endpoint running at `localhost:8002`
- I've compiled my WebAssembly function file to `/tmp/do_something.wasm`
- I want to upload this function to user `demo` and function name `cool_func`

I can execute:

```
curl -X PUT http://localhost:8002/f/demo/cool_func/ -T /tmp/do_something.wasm
```

