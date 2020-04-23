#pragma once

#include "TCPMessage.h"

#include <vector>
#include <atomic>
#include <thread>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <util/exception.h>


namespace tcp {
    class TCPServer {
    public:
        TCPServer(int portIn, long timeoutMillisIn);

        void start();

        void respond(TCPMessage *request, TCPMessage *response);

        void noResponse(TCPMessage *request);

        void close() const;

        virtual TCPMessage *handleMessage(TCPMessage *request) = 0;

    private:
        int port;
        int serverSocket;
        struct sockaddr_in serverAddress{};

        long timeoutMillis;
    };

    class TCPTimeoutException: public util::FaasmException {
    public:
        explicit TCPTimeoutException(std::string message): FaasmException(std::move(message)) {

        }
    };
}