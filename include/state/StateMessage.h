#pragma once

#include <cstdint>
#include <string>

#include <tcp/TCPMessage.h>


#define STATE_PORT 8003

namespace state {
    enum StateMessageType {
        ERROR_RESPONSE,
        OK_RESPONSE,
        STATE_DELETE,
        STATE_LOCK,
        STATE_PULL,
        STATE_PULL_CHUNK,
        STATE_PULL_RESPONSE,
        STATE_PULL_CHUNK_RESPONSE,
        STATE_PUSH,
        STATE_PUSH_CHUNK,
        STATE_SIZE,
        STATE_SIZE_RESPONSE,
        STATE_UNLOCK,
        TIMEOUT_RESPONSE,
    };

    struct StateMessage {
        int keyLen;
        char *key;
        int valueLen;
        uint8_t *value;
    };

    size_t getTCPMessageDataOffset(const std::string &user, const std::string &key);

    tcp::TCPMessage *buildStateTCPMessage(
            int msgType,
            const std::string &user,
            const std::string &key,
            size_t dataSize
    );
}