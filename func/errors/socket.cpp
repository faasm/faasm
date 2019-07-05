#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

FAASM_MAIN_FUNC() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr addr;

    // Create an invalid fd
    sockfd += 1;

    // Now try to connect
    int res = connect(sockfd, &addr, sizeof(addr));
    if (res < 0) {
        // This is expected
        return 1;
    }

    printf("ERROR - shouldn't be able to connect to invalid socket %i\n", sockfd);
    return 0;
}
