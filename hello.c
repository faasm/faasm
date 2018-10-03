#include "faasm.h"

int exec(struct FaasmMemory *memory) {
    memory.output = "Hello faasm!";

    return 0;
}
