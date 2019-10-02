#include "faasm/faasm.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <faasm/input.h>

/**
 * Takes in url as argument and returns content as output
 */
FAASM_MAIN_FUNC() {
    struct addrinfo *hostInfo, hints;

    const char *url = faasm::getStringInput("");

    hints.ai_family = AF_INET;

    printf("Calling addrinfo for %s\n", url);
    getaddrinfo(url, "80", &hints, &hostInfo);

    printf("Opening socket\n");
    int sockfd = socket(hostInfo->ai_family, hostInfo->ai_socktype, hostInfo->ai_protocol);

    printf("Connecting\n");
    connect(sockfd, hostInfo->ai_addr, hostInfo->ai_addrlen);

    printf("Sending request\n");
    char header[60 + strlen(url)];
    strcat(header, "GET /index.html HTTP/1.1\r\nHost: ");
    strcat(header, url);
    strcat(header, "\r\n\r\n");

    send(sockfd, header, strlen(header), 0);

    // Receive data into buffer and add null terminator
    printf("Receiving\n");
    char buffer[2056];
    ssize_t byte_count = recv(sockfd, buffer, 2056, 0);

    printf("Received %li bytes of data\n", byte_count);
    printf("%.*s", (int) byte_count, buffer);

    // Copy to output
    faasmSetOutput((uint8_t *) buffer, (size_t) byte_count);

    return 0;
}