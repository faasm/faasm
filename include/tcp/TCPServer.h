#pragma once

#include "TCPMessage.h"

#include <vector>
#include <atomic>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <util/exception.h>

#define POLL_COUNT 1000

namespace tcp {
    class TCPServer {
    public:
        TCPServer(int portIn, long timeoutMillisIn);

        int poll();

        void close() const;

        virtual TCPMessage *handleMessage(TCPMessage *request) = 0;

    private:
        int port;
        int serverSocket;
        struct sockaddr_in serverAddress{};
        struct pollfd pollFds[POLL_COUNT];
        int nFds = 1;

        long timeoutMillis;
    };

    class TCPFailedException : public util::FaasmException {
    public:
        explicit TCPFailedException(std::string message) : FaasmException(std::move(message)) {

        }
    };

    class TCPTimeoutException : public util::FaasmException {
    public:
        explicit TCPTimeoutException(std::string message) : FaasmException(std::move(message)) {

        }
    };
}