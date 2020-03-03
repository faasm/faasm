
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