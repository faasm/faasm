#include "faasm.h"

int exec(struct FaasmMemory *memory) {
    char* message = "Hello faasm!";

    // Output to be returned to the caller must be serialised 
    // and put into the relevant location
    memcpy(memory->output, message, strlen(message));

    return 0;
}
