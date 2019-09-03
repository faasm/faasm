#include "faasm/core.h"

// This is the default zygote implementation. If the user defines a zygote, it
// will beat this in the resolution order
__attribute__((noinline))
int _faasm_zygote() {
    // Do nothing
    return 0;
}
