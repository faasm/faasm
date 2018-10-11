#include "faasm.h"

int exec(struct FaasmMemory *memory) {
    char *message = "Hello faasm!";
    memory->output = (uint8_t*) message;

    return 0;
}
