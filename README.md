# Faasm

Faasm is a serverless system focused on performance and security, using WebAssembly to run users' code.
The features of WebAssmebly mean we can trust users' code and maintain good isolation with very lightweight mechanisms.
For more details on the isolation approach, see the [wiki page](https://github.com/lsds/faasm/wiki/isolation).

This lightweight isolation enables excellent performance and opens the door to features not possible in a more strongly
isolated environment (e.g. inter-function communication, shared state).

The underlying WebAssembly execution is handled by [WAVM](https://github.com/WAVM/WAVM), which is well worth
checking out.

More detail on the internals, development and deployment is held in the [wiki](https://github.com/lsds/faasm/wiki).

A preconfigured, Dockerised toolchain is provided for compiling Faasm functions. Details are given below.
This is based on a basic [Dockerised WASM toolchain](https://github.com/Shillaker/wasm-toolchain) with a customised
libc to implement syscalls and hooks into the Faasm runtime.

# Quick Start

To run the project you can use the `docker-compose.yml` file in the root of the project, i.e. 

```
docker-compose up -d
```

Then curl one of the example functions:

```
curl -X POST http://localhost:8001/f/demo/echo -d "Hello!"
```

And you should see your message echoed back.

## Compiling a simple function

The `hello.cpp` file in the root of this directory shows a basic faasm function.
To compile this to WebAssembly you can run the following from the project root:

```
# Start the wasm toolchain container
docker run -v $(pwd):/work -w /work -it faasm/toolchain

# From inside the container, compile the hello.c file
/toolchain/bin/clang++ --target=wasm32-unknown-unknown-wasm --sysroot=/toolchain/sysroot -o hello.wasm -I include/faasm hello.cpp

# Drop out of the container
exit

# Upload the function
curl -X PUT http://localhost:8001/f/dummy/hello -T hello.wasm

# Invoke the function
curl -X POST http://localhost:8001/f/dummy/hello
```

# Usage

## Functions

Each function is associated with a user and has a function name. It will have two URLs:

- Synchronous - `<faasm_host>/f/<user>/<function>/`
- Asynchronous - `<faasm_host>/fa/<user>/<function>/`

By `POST`ing to these URLs we can invoke the function. POSTed data forms the input data for the function call.

For example, with the faasm endpoint at `localhost:8001`, the `echo` function owned by `demo` can be run with:

```
curl -X POST http://localhost:8001/f/demo/echo -d "hello faasm"
```

The code for this function can be found in `examples/echo.cpp`.

## Writing Functions

Functions consist of an `exec` entrypoint which takes a pointer to a `FaasmMemory` object. This object is the
interface for interacting with the Faasm runtime.

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Do something

        return 0;
    }
}
```

Some example functions can be found in the `examples` directory.

### `FaasmMemory`

The `FaasmMemory` class allows Faasm functions to interact with the system. It has the following methods:

- `getInput()` - this returns an array containing the input data to the function
- `setOutput()` - this allows functions to return output data to the caller
- `chainFunction()` - this allows one function to invoke others
- `getStateSize()` - returns the size (in bytes) of the current state associated with the given key 
- `readState()` and `writeState()` - allows functions to read/ write key/value state
- `readStateOffset()` and `writeStateOffset()` - allows functions to read/ write at specific points in existing state (e.g. updating a subsection of an array)

## Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user).

To do this, the `chainFunction()` method on the `FaasmMemory` instance can be called. For my function to invoke
the function `foo`, (also owned by me), it can do the following:

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        uint8_t funcData[] = {1, 2, 3, 4};
        int dataLength = 4;
        const char* funcName = "foo";

        memory->chainFunction(funcName, funcData, dataLength);

        return 0;
    }
}
```

`chainFunction()` can be called multiple times in one function. Once the original function has completed, these
calls will go back through the main scheduler and be executed.

## State

All of a users' functions have access to shared state. This state is implemented as a simple key-value store
and accessed by the functions `readState` and `writeState` on the `FaasmMemory` object. Values read and written
to this state must be byte arrays.

A function accessing state will look something like:

```
#include "faasm/faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "my_state_key";

        // Get initial state size (if not known)
        long stateSize = memory->getStateSize(key);

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

`FaasmMemory` also exposes the `readStateOffset` and `writeStateOffset` functions, which allow reading and writing
subsections of byte arrays stored against keys in the key-value store.

For example, if I have an array of 100 bytes stored in memory, I can read bytes 15-20, then update them separately.

This can be useful for implementing distributed iterative algorithms.

### State example

An example implementing a simple counter can be found in `examples/increment.cpp`. It can be called with:

```
curl http://localhost:8001/f/demo/increment/ -X POST
```

On successive calls this will add 1 to a value stored in shared state, returning something like:

```
Counter: 1
Counter: 2
Counter: 3
...
```

### Uploading state

If you need to prepopulate state for your functions you can use the state upload endpoint. This can be called
with:

```
# Format /s/<username>/<key>
curl http://localhost:8001/s/user123/my_key -X PUT -d "your data"
```

This state can then be accessed in your functions using the specified key. For larger state, you can also use:

```
curl http://localhost:8001/s/user123/my_key -X PUT -T /tmp/my_state_file
```

Where `/tmp/my_state_file` contains binary data you wish to be inserted at your specified key.

## Uploading Functions

To upload a function you can use `curl` to send a PUT request to the synchronous URL for the given function.
For example:

- I have a Faasm endpoint running at `localhost:8001`
- I've compiled my WebAssembly function file to `/tmp/do_something.wasm`
- I want to upload this function to user `demo` and function name `cool_func`

I can execute:

```
curl http://localhost:8001/f/demo/cool_func/ -X PUT -T /tmp/do_something.wasm
```


