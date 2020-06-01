#include <faasm/faasm.h>

// This is a dummy placeholder that should only get executed
// for testing purposes
FAASM_MAIN_FUNC() {
    printf("I should not be getting executed\n");
    return 0;
}