#include "faasm.h"
#include <unistd.h>

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    printf("Sleeping for 10 seconds\n");

    unsigned int seconds = 10;
    sleep(seconds);

    return 0;
}