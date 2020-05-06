#include <algorithm>
#include <util/macros.h>
#include "TCPMessage.h"

namespace tcp {
    void freeTcpMessage(TCPMessage *msg) {
        if(msg != nullptr) {
            delete[] msg->buffer;
            delete msg;
        }
    }

    size_t tcpMessageLen(TCPMessage *msg) {
        return sizeof(TCPMessage) + msg->len;
    }
    
    uint8_t *tcpMessageToBuffer(TCPMessage *msg) {
        // Overall buffer length
        size_t bufferLen = tcpMessageLen(msg);
        auto buffer = new uint8_t[bufferLen];

        // Size of struct
        size_t structSize = sizeof(TCPMessage);

        // Copy struct in
        auto bytes = BYTES(msg);
        std::copy(bytes, bytes + structSize, buffer);

        // Copy data buffer after
        std::copy(msg->buffer, msg->buffer + msg->len, buffer + structSize);

        return buffer;
    }

    TCPMessage *tcpMessageFromBuffer(uint8_t* buffer) {
        auto tcpMessage = reinterpret_cast<TCPMessage *>(buffer);
        tcpMessage->buffer = buffer + sizeof(TCPMessage);

        return tcpMessage;
    }
}