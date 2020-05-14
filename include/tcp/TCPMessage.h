#pragma once

#include <cstddef>
#include <cstdint>
#include <util/exception.h>


namespace tcp {
    struct TCPMessage {
        int type;
        size_t len;
        uint8_t *buffer;
    };

    void freeTcpMessage(TCPMessage *msg);

    size_t tcpMessageLen(TCPMessage *msg);

    uint8_t *tcpMessageToBuffer(TCPMessage *msg);

    TCPMessage *tcpMessageFromBuffer(uint8_t* buffer);

    TCPMessage *receiveTcpMessage(int fd);

    class TCPSenderClosedException : public util::FaasmException {
    public:
        TCPSenderClosedException(std::string message) : FaasmException(std::move(message)) {

        }
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