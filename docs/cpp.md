## Compiling a C++ function

C++ functions are built with CMake and held in the [func](../func) directory. 

A simple hello world function exists at [hello.cpp](../func/demo/hello.cpp).

From the Faasm CLI, you can compile, upload and invoke the `hello.cpp` 
function with:

```bash
inv compile demo hello
inv upload demo hello
inv invoke demo hello
```

You should then see the response `Hello faasm!`.

# Writing functions

Faasm aims to be uninvasive, allowing code to run natively _and_ in a serverless context. 
To do this in C/ C++ functions we use a set of Faasm macros. These macros allow code to 
be compiled with a standard toolchain and run natively, and with the Faasm toolchain, 
and run in a serverless context.

The outline of this looks like:

```c++
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    // Do something

    return 0;
}
```

Although for very simple functions, a standard `int main()` will also still work.

## C++ API

Faasm provides a simple C++ wrapper library around the [Faasm host interface](host_interface.md).
Some of the methods in this wrapper are:

- `faasmGetInput()` - allows functions to retrieve their input data
- `faasmSetOutput()` - this allows functions to return output data to the caller
- `faasmChainFunction()` - this allows one function to invoke others
- `faasmAwaitCall()` - waits for a chained function invocation to finish
- `faasmReadState()` and `writeState()` - allows functions to read/ write key/value state
- `faasmReadStateOffset()` and `faasmWriteStateOffset()` - allows functions to read/ write at specific points in existing state (e.g. updating a subsection of an array)

## Chaining

"Chaining" is when one function makes a call to another function (which must be owned by the same user). 
There are two supported methods of chaining, one for invoking totally separate Faasm functions, the 
other for automatically parallelising functions in the same piece of code (useful for porting legacy applications).

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

## OpenMP and MPI

See the [OpenMP](openmp.md) and [MPI](mpi.md) docs for further C/C++ APIs.