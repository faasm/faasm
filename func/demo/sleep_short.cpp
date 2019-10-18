#include "faasm/faasm.h"
#include "faasm/input.h"

#include <unistd.h>
#include <stdio.h>

#define SLEEP_TIME_SECONDS 25

FAASM_MAIN_FUNC() {
    const char* input = faasm::getStringInput("No input");
    printf("Sleep input: %s\n", input);

    sleep(SLEEP_TIME_SECONDS);
    return 0;
}
