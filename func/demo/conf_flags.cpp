#include <faasm/faasm.h>
#include <stdio.h>

FAASM_MAIN_FUNC() {
    unsigned int on = getConfFlag("ALWAYS_ON");
    unsigned int off = getConfFlag("ALWAYS_OFF");

    if (on != 1 || off != 0) {
        printf("On and off not as expected (%u and %u)\n", on, off);
        return 1;
    }

    return 0;
}
