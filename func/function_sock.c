#include "faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * Tries to open a socket connection
 */
EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    int sock = socket(AF_INET , SOCK_STREAM , 0);

    printf("Function sees socket %i\n", sock);

    return 0;
}