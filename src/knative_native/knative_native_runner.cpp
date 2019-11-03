#include <faasm/faasm.h>

// This is a dummy placeholder that ought not to get executed
FAASM_MAIN_FUNC() {
    printf("I should not be getting executed\n");
    return 0;
}