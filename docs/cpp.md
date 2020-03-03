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
