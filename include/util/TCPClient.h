#pragma once

#include <vector>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define RECEIVE_SIZE 256

namespace util {

    class TCPClient {
    public:
        TCPClient(std::string host, int port);

        void sendBytes(uint8_t *buffer, int bufferLen);

        size_t receiveBytes(uint8_t *buffer, int bufferLen);

        void exit();
    private:
        std::string host;
        int port;

        int clientSocket;
    };
}

