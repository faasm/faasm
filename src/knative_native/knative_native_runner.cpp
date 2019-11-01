#include <faasm/faasm.h>

// This is only needed for executing C++ functions natively.
// Python funcs can just sit there using the basic endpoint
void exec() {
    printf("Dummy exec\n");
}