#pragma once

#include "TCPMessage.h"

#include <vector>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace tcp {
    class TCPServer {
    public:
        explicit TCPServer(int portIn);

        TCPMessage *accept() const;

        void respond(TCPMessage *request, TCPMessage *response);

        void noResponse(TCPMessage *request);

        void close() const;

    private:
        int port;
        int serverSocket;
        struct sockaddr_in serverAddress{};

        std::vector<std::thread> threads;
    };

}