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

#define INITIAL_POLLFD_CAPACITY 1000

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

        // TODO - more performant to have a vector or a list here?
        // Need to reguarly pass a pointer to the underlying data to poll()
        // but also need to remove elements at arbitrary locations. We
        // assume that the ability to access a pointer to the underlying data
        // (which must happen on every poll) takes precedence.
        std::vector<pollfd> pollFds;

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