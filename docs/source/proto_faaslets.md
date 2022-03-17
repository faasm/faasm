# Proto-faaslets

Proto-Faaslets are a way to restore the execution state of a function without
rerunning the code that got it to that state.

Proto-Faaslets mean we can snapshot a function when it's running, then restore
it later, resuming its execution. The same proto-function can be restored
multiple times and across different hosts.

## Reducing cold starts

Proto-Faaslets can be used to reduce function cold starts by:

- Running initialisation code _once_ when the function is uploaded
- Creating a proto-function _after_ this function has been run
- Restoring all subsequent function calls from this snapshot

Any slow or resource-intensive initialisation that would otherwise be performed
on every call can be captured in this proto-function snapshot, thus repeatedly
saving on overheads.

### Defining a proto-function

Proto-Faaslets can be defined in C/C++ with the `FAASM_ZYGOTE` macro:

```c++
#include "faasm/faasm.h"

int myGlobal;

FAASM_ZYGOTE() {
    // Run once
    myGlobal = 5;

    return 0;
}

int main(int argc, char* argv[]) {
    // Context available to all subsequent function calls
    printf("My global = %i\n", myGlobal);

    return 0;
}
```

## Cross-host migration

Proto-Faaslets are also used in Faasm to migrate functions across hosts.
