#pragma once

#include "TCPMessage.h"

#include <vector>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define RECEIVE_SIZE 256

namespace tcp {

    class TCPClient {
    public:
        TCPClient(std::string host, int port);

        void sendMessage(TCPMessage *msg);

        TCPMessage *recvMessage();

        TCPMessage *recvMessage(size_t dataSize);

        void exit();
    private:
        std::string host;
        int port;

        int clientSocket;
    };
}

