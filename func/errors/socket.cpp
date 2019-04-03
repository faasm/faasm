#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr addr;

        // Create an invalid fd
        sockfd += 1;
        printf("Invalid socket fd: %i\n", sockfd);

        // Now try to connect
        connect(sockfd, &addr, sizeof(addr));

        return 0;
    }
}
