#pragma once

#include <cstddef>
#include <cstdint>


namespace tcp {
    enum TCPMessageType {
        STANDARD,
    };

    struct TCPMessage {
        int _fromSocket;
        TCPMessageType type;
        size_t len;
        uint8_t *buffer;
    };

    size_t tcpMessageLen(TCPMessage *msg);

    uint8_t *tcpMessageToBuffer(TCPMessage *msg);

    TCPMessage *tcpMessageFromBuffer(uint8_t* buffer);
}