#include "faasm.h"

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * Tries to open a socket connection
 */
int exec(struct FaasmMemory *memory) {
    int sock = socket(AF_INET , SOCK_STREAM , 0);

    printf("Function sees socket %i\n", sock);

    struct hostent *lh = gethostbyname("localhost");

    if (lh)
        printf(lh->h_name);
    else
        herror("gethostbyname");

    return 0;
}