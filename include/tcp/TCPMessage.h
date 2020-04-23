#pragma once

#include <cstddef>
#include <cstdint>


namespace tcp {
    enum TCPMessageType {
        STANDARD,
        STATE_SIZE,
        STATE_GET,
        STATE_SET,
    };

    struct TCPMessage {
        TCPMessageType type;
        size_t len;
        uint8_t *buffer;
    };

    void freeTcpMessage(TCPMessage *msg);

    size_t tcpMessageLen(TCPMessage *msg);

    uint8_t *tcpMessageToBuffer(TCPMessage *msg);

    TCPMessage *tcpMessageFromBuffer(uint8_t* buffer);
}