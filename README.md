<div align="center">
<img src="https://raw.githubusercontent.com/lsds/Faasm/master/faasm_logo.png"></img>
</div>

# Faasm [![Tests](https://github.com/lsds/Faasm/workflows/Tests/badge.svg?branch=master)](https://github.com/lsds/Faasm/actions)  [![License](https://img.shields.io/github/license/lsds/Faasm.svg)](https://github.com/lsds/Faasm/blob/master/LICENSE.md)  [![Release](https://img.shields.io/github/release/lsds/Faasm.svg)](https://github.com/lsds/Faasm/releases/)  [![Contributors](https://img.shields.io/github/contributors/lsds/Faasm.svg)](https://github.com/lsds/Faasm/graphs/contributors/)

Faasm is a high-performance stateful serverless runtime. The goal of the project is enabling fast, efficient serverless big data.

Faasm provides multi-tenant isolation, but also lets functions share regions of memory. These shared memory regions give low-latency concurrent access to data, supporting high-performance distributed serverless applications.

By running WebAssembly, Faasm combines software fault isolation with standard OS tooling to provide security and resource isolation guarantees at low cost. Functions run side-by-side as threads of a single runtime process, with low overheads and fast boot times. The underlying WebAssembly execution and code generation is handled by [WAVM](https://github.com/WAVM/WAVM), an excellent server-side WebAssembly VM. 

Faasm supports C/C++ natively and extends support to dynamic languages such as Python by compiling the language runtime itself to WebAssembly. The Python support is based heavily on the work of the [Pyodide](https://github.com/iodide-project/pyodide) project, with custom C-extensions and decorators in [Pyfaasm](https://github.com/Shillaker/pyfaasm).

Faasm uses a custom host interface to give functions access to state and interact with the runtime. Larger applications can be constructed by composing multiple functions together dynamically in chains. The Faasm scheduler ensures these functions execute close to their required data, reducing unnecessary duplication and overhead.

Faasm is a runtime, intended for integration into other serverless platforms. The primary integration is with [Knative](https://knative.dev/).

# Quick start

You can start a simple Faasm runtime using the `docker-compose.yml` file in the root of the project. This creates a couple of worker instances as well as an upload endpoint for receiving functions and state. There is also a Redis container used for communication between the workers.

You can start it by running:

```bash
# Single worker
docker-compose up

# Three workers
docker-compose up --scale worker=3
```

Note that the first time you run the local set-up it will generate some machine code specific to your host. This is
stored in the `machine-code` directory in the root of the project and reused on subsequent runs.

## Compiling a C++ function

C++ functions are built with CMake and held in the `func` directory. `demo/hello.cpp` is a simple hello world function.

The Faasm toolchain is packaged in the `faasm/toolchain` container and can be run with the `bin/toolchain.sh` script, i.e.:

```bash
./bin/toolchain.sh
```

This container has all the tooling ready to use. To compile and upload the `hello` function you an run:

```bash
inv compile demo hello
inv upload demo hello
```

You can invoke the function as follows:

```bash
inv invoke demo hello
```

You should then see the response `Hello faasm!`.

## Running a Python function

An example Python function is found at `func/python/hello.py`. This can be uploaded with:

```bash
inv upload --py python hello
```

And invoke with:

```bash
inv invoke --py python hello
```

This should give a message and the version of Python being run.

# Writing functions

Faasm aims to be uninvasive, allowing code to run natively _and_ in a serverless context. This is important for local development and testing as well as porting existing applications.

## C++

In C++ functions make use of the Faasm macros. These macros mean the code can be compiled with a standard toolchain and run natively, but when compiled with the Faasm toolchain, will run in a serverless context.

```c++
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    // Do something

    return 0;
}
```

Some example functions can be found in the `func/demo` directory.

## C++ API

Faasm allows users functions to interact with the underlying system to accomplish a number of things e.g. accessing input and output, interacting with distributed state, and invoking other functions.

Some of the methods include:

- `faasmGetInput()` - allows functions to retrieve their input data
- `faasmSetOutput()` - this allows functions to return output data to the caller
- `faasmChainFunction()` - this allows one function to invoke others
- `faasmAwaitCall()` - waits for a chained function invocation to finish
- `faasmReadState()` and `writeState()` - allows functions to read/ write key/value state
- `faasmReadStateOffset()` and `faasmWriteStateOffset()` - allows functions to read/ write at specific points in existing state (e.g. updating a subsection of an array)

## Python API

Python functions interact with the Faasm API via [pyfaasm](https://github.com/Shillaker/pyfaasm), a module with a C-extension providing the relevant bindings to the C++ API.

## Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user). There are two supported methods of chaining, one for invoking totally separate Faasm functions, the other for automatically parallelising functions in the same piece of code (useful for porting legacy applications).

### Chaining a function

Multiple functions can be defined in the same file, invoke each other and await results. For example:

```c++
#include "faasm/faasm.h"
#include <vector>

// Define some function to be chained
FAASM_FUNC(funcOne, 1) {
    return 0;
}

// Define another function to be chained
FAASM_FUNC(funcTwo, 2) {
    return 0;
}

// Define the main function
FAASM_MAIN_FUNC() {
    // Chain calls to the other functions
    int callOneId = faasmChainThis(1);
    int callTwoId = faasmChainThis(2);

    // Await results
    faasmAwaitCall(callOneId);
    faasmAwaitCall(callTwoId);

    return 0;
}
```

In Python this looks like:

```python
from pyfaasm.code import await_call, chain_this, faasm_func, faasm_main

@faasm_func(1)
def func_one():
    pass

@faasm_func(2)
def func_two():
    pass

@faasm_main
def main_func():
    call_one = chain_this(1)
    call_two = chain_this(2)

    await_call(call_one)
    await_call(call_two)
```

Chaining can also be done across functions defined separately, e.g. in C++:

```c++
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    // Chain a call to my other function named "other-func"
    int callId = faasmChainFunction("other-func");
    faasmAwaitCall(callId);

    return 0;
}
```

## State

All of a users' functions have access to shared state. This state is implemented as a simple key-value store and accessed by the functions `faasmReadState` and `faasmWriteState`. Values read and written to this state must be byte arrays.

State can be dealt with in a read-only or lock-free manner (and shared in regions of shared memory with colocated functions), or via synchronous distributed locks.

A function accessing state will look something like:

```c++
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    const char *key = "my_state_key";

    // Read the state into a buffer
    long stateSize = 123;
    uint8_t *myState = new uint8_t[stateSize];
    faasmReadState(key, newState, stateSize);

    // Do something useful, modify state

    // Write the updated state
    faasmWriteState(key, myState, stateSize);

    return 0;
}
```

### Offset state

Faasm also exposes the `faasmReadStateOffset` and `faasmWriteStateOffset` functions, which allow reading and writing subsections of byte arrays stored against keys in the key-value store.

For example, if I have an array of 100 bytes stored in memory, I can read bytes 15-20, then update them separately.

This can be useful for implementing distributed iterative algorithms.

### Uploading state

If you need to prepopulate state for your functions you can use the state upload endpoint. This can be called with:

```bash
# Format /s/<username>/<key>
curl http://localhost:8002/s/user123/my_key -X PUT -d "your data"
```

This state can then be accessed in your functions using the specified key. For larger state, you can also upload a file:

```bash
curl http://localhost:8002/s/user123/my_key -X PUT -T /tmp/my_state_file
```

Where `/tmp/my_state_file` contains binary data you wish to be inserted at your specified key.

## Proto-functions

Proto-functions are a way to reduce the initialisation time of functions. They are a chunk of code that executes once and is then used to spawn all subsequent function invocations. The complete state of the function after proto-function execution is duplicated for all subsequent function invocations.

The proto-function should be idempotent as it may be run more than once.

Proto-function code is marked up with the `FAASM_ZYGOTE` macro:

```c++
#include "faasm/faasm.h"

int myGlobal;

FAASM_ZYGOTE() {
    // Run once
    myGlobal = 5;

    return 0;
}

FAASM_MAIN_FUNC() {
    // Context available to all subsequent function calls
    printf("My global = %i\n", myGlobal);

    return 0;
}
```

# Scheduling and Work Sharing

Faasm workers schedule tasks via a standard work-sharing approach. This means function calls are distributed randomly, and workers will hand them off to another worker if that worker is better suited to execute it. This means functions will always be run by the same worker (hence able to share local in-memory state), provided resources are available.

In auto-scaling environments such as KNative, the workers will auto-scale to meet demand.

# Integrations

Faasm's recommended integration is with [KNative](https://cloud.google.com/knative/), although it also works with AWS Lambda and IBM Cloud Functions. For more information consult the `docs` folder.
