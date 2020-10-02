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

Faasm aims to be uninvasive, allowing code to run natively _and_ in a serverless
context. This means the simplest Faasm function looks like:

```c++
int main(int argc, char* argv[]) {
    // Do something

    return 0;
}
```

## C++ API

Faasm provides a simple C++ wrapper library around the [Faasm host
interface](host_interface.md).  Some of the methods in this wrapper are:

- `faasmGetInput()` - allows functions to retrieve their input data
- `faasmSetOutput()` - this allows functions to return output data to the caller
- `faasmChain()` - this allows one function to invoke others
- `faasmAwaitCall()` - waits for a chained function invocation to finish
- `faasmReadState()` and `writeState()` - allows functions to read/ write
  key/value state
- `faasmReadStateOffset()` and `faasmWriteStateOffset()` - allows functions to
  read/ write at specific points in existing state (e.g. updating a subsection
  of an array)

They are found in the header `faasm/faasm.h`.

## Chaining

"Chaining" is when one function makes a call to another function (which must be
owned by the same user).  There are two supported methods of chaining, one for
invoking totally separate Faasm functions, the other for automatically
parallelising functions in the same piece of code (useful for porting legacy
applications).

### Chaining a function

Multiple functions can be defined in the same file, invoke each other and await
results. For example:

```c++
#include "faasm/faasm.h"
#include <vector>

// Define some function to be chained
int funcOne() {
    return 0;
}

// Define another function to be chained
int funcTwo() {
    return 0;
}

// Define the main function
int main(int argc, char* argv[]) {
    // Chain calls to the other functions
    int callOneId = faasmChain(funcOne);
    int callTwoId = faasmChain(funcTwo);

    // Await results
    faasmAwaitCall(callOneId);
    faasmAwaitCall(callTwoId);

    return 0;
}
```

Chaining can also be done by name across functions defined separately, e.g.:

```c++
#include "faasm/faasm.h"

int main(int argc, char* argv[]) {
    // Chain a call to my other function named "other-func"
    int callId = faasmChainNamed("other-func");
    faasmAwaitCall(callId);

    return 0;
}
```

## OpenMP and MPI

See the [OpenMP](openmp.md) and [MPI](mpi.md) docs for further C/C++ APIs.
