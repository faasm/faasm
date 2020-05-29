#include "faasm/faasm.h"
#include "faasm/input.h"

#include <unistd.h>
#include <stdio.h>

#define SLEEP_TIME_SECONDS 25

FAASM_MAIN_FUNC() {
    printf("Sleeping for: %is\n", SLEEP_TIME_SECONDS);

    sleep(SLEEP_TIME_SECONDS);
    return 0;
}
