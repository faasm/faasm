#include "faasm.h"

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


/**
 * Tries to open a socket connection
 */
int exec(struct FaasmMemory *memory) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    
//    struct sockaddr s;
//    s.sa_data[0] = 25;
//    s.sa_data[1] = 4;
//    s.sa_data[2] = 16;
//
//    bind(sock, &s, sizeof(s));

    uint8_t buf[] = {12, 10, 4, 22, 10, 8};
    
    send(sock, buf, 6, 0);

    //    int sock = socket(AF_INET , SOCK_STREAM , 0);
//
//    printf("Function sees socket %i\n", sock);
//
//    struct hostent *lh = gethostbyname("localhost");
//
//    printf(lh->h_name);

    return 0;
}