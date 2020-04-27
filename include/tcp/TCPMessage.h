#pragma once

#include <cstddef>
#include <cstdint>


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
}