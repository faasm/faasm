#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    const char *message = "Hello faasm!";

    faasmSetOutput((uint8_t *) message, 12);

    return 0;
}
