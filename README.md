# Faasm

[![CircleCI](https://circleci.com/gh/lsds/Faasm.svg?style=svg)](https://circleci.com/gh/lsds/Faasm)

Faasm is a high-performance multi-tenant serverless runtime. It is intended for integration into
other larger serverless platforms, not as a stand-alone system.

By using WebAssembly to run users' code, we can combine software fault isolation with 
standard OS tooling to provide strong security and resource isolation guarantees at low cost.

This lightweight isolation enables excellent performance and allows sharing of data between 
colocated functions through shared memory. We can also snapshot and restore a function's memory to
reduce initialisation time.

Faasm currently supports C/C++ and Python. C/C++ are compiled using the stanard LLVM WebAssembly toolchain,
while Python support is offered by compiling CPython itself to WebAssembly.

Faasm uses its own custom host interface to allow functions to interact with the runtime, as well as
provide secure access to networking and state. This is not dissimilar to WASI, but the Faasm interface focuses
solely on a server-side POSIX environment, thus exposes more of the underlying host functionality.

The WebAssembly execution and code generation is handled by [WAVM](https://github.com/WAVM/WAVM),
an excellent server-side WebAssembly VM. The Python support is thanks to the
[Pyodide](https://github.com/iodide-project/pyodide) project.

This is primarily a research project. Other serverless WebAssembly runtimes are available.

# Quick start

You can start a simple Faasm runtime using the `docker-compose.yml` file in the root of the project. This
creates a simple serverless system on the local machine with an `edge` container for receiving requests, an
`upload` container for handling uploads of functions and states, and a `worker` container holding the runtime
itself. There is also a `redis` container used for communication between the others.

You can start it by running:

```
docker-compose up
```

## Compiling a C++ function

C++ functions are built with CMake and held in the `func` directory. `demo/hello.cpp` is a simple hello world
function.

The Faasm toolchain is packaged in the `faasm/toolchain` container and can be run with the `bin/toolchain.sh`
script, i.e.:

```
./bin/toolchain.sh
```

This container has all the tooling ready to use. To compile and upload the `hello` function you an run:

```
inv compile --func=hello
inv upload demo hello
```

You can invoke the function as follows:

```
inv invoke demo hello
```

You should then see the response `Hello faasm!`.

## Running a Python function

As mentioned above, Python functions are handled by executing the code in a WebAssembly-compiled version of
CPython. Every function is running the same underlying WebAssembly module, just dynamically loading a different
Python file. As a result, the commands are slightly different.

An example Python function is found at `funcs/python/hello.py`.

```
inv py-upload python hello
```

And invoke with:

```
inv py-invoke python hello
```

# Writing functions

## C++

C++ functions make use of the Faasm macros. These macros mean the code can be compiled and run natively, but
when compiled with the toolchain, will run on Faasm.

```
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    // Do something

    return 0;
}
```

Some example functions can be found in the `func/demo` directory.

## The Faasm API

Faasm allows users functions to interact with the underlying system to accomplish a number of things, mainly
accessing input and providing output, interacting with distributed state, and invoking other functions.

Some of the methods include:

- `faasmGetInput()` - allows functions to retrieve their input data
- `faasmSetOutput()` - this allows functions to return output data to the caller
- `faasmChainFunction()` - this allows one function to invoke others
- `faasmReadState()` and `writeState()` - allows functions to read/ write key/value state
- `faasmReadStateOffset()` and `faasmWriteStateOffset()` - allows functions to read/ write at specific points in existing state (e.g. updating a subsection of an array)
- `faasmCreateSnap()` and `faasmRestoreSnap()` - creates/ restores a given memory snapshot for this function

## Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user).
There are two supported methods of chaining, one for invoking totally separate Faasm functions, the other for
automatically parallelising functions in the same piece of code (useful for porting legacy applications).

### Chaining a different Faasm function

To call different Faasm functions, `faasmChainFunction()` can be used.
For my function to invoke the functions `foo` and `bar` (also owned by me),
it can do the following:

```
#include "faasm/faasm.h"
#include <vector>

FAASM_MAIN_FUNC() {
    int fooCallId = faasmChainFunction("foo");
    int barCallId = faasmChainFunction("bar");

    // Wait for both to complete (if you want)
    faasmAwaitCall(fooCallId);
    faasmAwaitCall(barCallId);

    return 0;
}
```

### Invoking a function from the same code

An alternative approach is to bundle all code into the same Faasm function,
then invoke bits of it in parallel.

```
#include "faasm/faasm.h"
#include <vector>

FAASM_MAIN_FUNC() {
    int callOneId = faasmChainThis(1);
    int callTwoId = faasmChainThis(2);

    // Wait for both to complete (if you want)
    faasmAwaitCall(callOneId);
    faasmAwaitCall(callTwoId);

    return 0;
}

FAASM_FUNC(funcOne, 1) {
    // Do something
    return 0;
}

FAASM_FUNC(funcTwo, 1) {
    // Do something different
    return 0;
}
```

## State

All of a users' functions have access to shared state. This state is implemented as a 
simple key-value store and accessed by the functions `faasmReadState` and `faasmWriteState`.
Values read and written to this state must be byte arrays.

A function accessing state will look something like:

```
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    const char *key = "my_state_key";

    long stateSize = 123;

    // Read the state into a buffer
    uint8_t *myState = new uint8_t[stateSize];
    faasmReadState(key, newState, stateSize);

    // Do something useful, modify state

    // Write the updated state
    faasmWriteState(key, myState, stateSize);

    return 0;
}
```

### Offset state

Faasm also exposes the `faasmReadStateOffset` and `faasmWriteStateOffset` functions,
which allow reading and writing subsections of byte arrays stored against keys in the 
key-value store.

For example, if I have an array of 100 bytes stored in memory, I can read bytes 15-20, 
then update them separately.

This can be useful for implementing distributed iterative algorithms.

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

# Integrations

Faasm aims to be pluggable so that it can integrate into other serverless platforms. This pluggability
includes the following:

- Function input (JSON or serialised protobuf objects)
- Function storage (S3 or NFS)
- Message queues (Redis or SQS)

## AWS Lambda

The Faasm runtime can be used with AWS Lambda, but requires some fiddles. This is because Faasm executes multiple
functions in the same container, whereas Lambda takes a one-function-per-container approach. To get around
this we can do the following:

- Use the Lambda C++ runtime to create a Faasm worker Lambda
- Have this Lambda dynamically load function code from S3 when needed
- Create "edge" Lambdas for each function
- Forward requests from edge Lambdas to the Faasm workers via an SQS queue

## Provisioning the Lambda environment

Setting up a Faasm deployment on Lambda can be done with the Ansible scripts in the `ansible` directory, namely:

```
cd ansible
ansible-playbook aws_lambda.yml
```

This requires that you have locally configured the AWS SDK. Note that this set-up needs to create an Elasticache
Redis instance which takes a while to provision. This Elasticache instance is also costly to maintain, so
_make sure you tear things down once done!_. This is done with:

```
ansible-playbook aws_teardown.yml
```

## Deploying functions to Lambda

_TODO_
