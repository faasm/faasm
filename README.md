# Faasm

Faasm is a serverless system focused on performance and security, using WebAssembly to run users' code. The features of WebAssmebly mean we can trust users' code and maintain good security and isolation with very lightweight mechanisms. For more details on the isolation approach, see the [wiki page](https://github.com/lsds/faasm/wiki/isolation).

This lightweight isolation enables excellent performance and opens the door to features not possible in a more strongly isolated environment (e.g. inter-function communication, shared state etc.).

The underlying WebAssembly execution is handled by [WAVM](https://github.com/WAVM/WAVM), which is well worth checking out.

More detail on the internals, development and deployment is held in the [wiki](https://github.com/lsds/faasm/wiki).

For compiling WebAssembly it is highly recommended you use the [Dockerised WASM toolchain](https://github.com/Shillaker/wasm-toolchain). This vastly simplifies the process.

# Quick Start

To run the project you can use the `docker-compose.yml` file in the root of the project, i.e. 

```
docker-compose up -d
```

Then curl one of the example functions:

```
curl -X POST http://localhost:8001/f/simon/echo -d "Hello!"
```

And you should see your message echoed back.

## Compiling a simple function

The `hello.c` file in the root of this directory shows a basic faasm function. To compile this to WebAssembly you can run the following (assuming you have Docker installed) from the project root:

```
# Start the wasm toolchain container
docker run -v $(pwd):/work -w /work -it shillaker/wasm-toolchain

# From inside the container, compile the hello.c file
/toolchain/bin/clang --target=wasm32-unknown-unknown-wasm --sysroot=/toolchain/sysroot -o hello.wasm -I include/faasm hello.c

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

For example, with the faasm endpoint at `localhost:8080`, the `echo` function owned by `simon` can be run with:

```
curl -X POST http://localhost:8080/f/simon/echo -d "hello faasm"
```

This function just returns its input so should give a response containing our input data (i.e. `hello faasm`).
The code can be found in `func/function_echo.c`.

## Writing Functions

The function API passes a number of pointers to functions to allocate memory regions. A convenience header is
provided in at `include/faasm/faasm.h`. Functions will look something like this:

```
#include "faasm.h"

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

