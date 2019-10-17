#include "faasm/faasm.h"
#include "faasm/counter.h"

#include <string>

#include <stdio.h>

FAASM_MAIN_FUNC() {
    const char *key = "incr_example";

    faasm::incrementCounter(key);
    faasmPushState(key);

    int count = faasm::getCounter(key);

    // Return message
    char output[13];
    sprintf(output, "Counter: %03i", count);
    faasmSetOutput((uint8_t *) output, 12);

    return 0;
}