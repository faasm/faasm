#pragma once

#include "StateKeyValue.h"

#include <cstdint>
#include <string>

#include <tcp/TCPMessage.h>


#define STATE_HOST "0.0.0.0"
#define STATE_PORT 8003

namespace state {
    enum StateMessageType {
        ERROR_RESPONSE,
        OK_RESPONSE,
        SHUTDOWN,
        STATE_APPEND,
        STATE_CLEAR_APPENDED,
        STATE_DELETE,
        STATE_LOCK,
        STATE_PULL,
        STATE_PULL_APPENDED,
        STATE_PULL_CHUNK,
        STATE_PULL_RESPONSE,
        STATE_PULL_CHUNK_RESPONSE,
        STATE_PULL_APPENDED_RESPONSE,
        STATE_PUSH,
        STATE_PUSH_CHUNK,
        STATE_PUSH_MANY_CHUNK,
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

    size_t getStateTcpMessageSize(const std::string &user, const std::string &key,size_t dataSize);

    std::pair<std::string, std::string> getUserKeyFromStateMessage(tcp::TCPMessage *msg);

    tcp::TCPMessage *buildStateSizeRequest(const std::string &user, const std::string &key);

    tcp::TCPMessage *buildStateSizeResponse(const std::string &user, const std::string &key, size_t stateSize);

    tcp::TCPMessage *buildStateDeleteRequest(const std::string &user, const std::string &key);

    size_t extractSizeResponse(const tcp::TCPMessage *msg);

    void extractPullAppendedData(const tcp::TCPMessage *msg, uint8_t *buffer);
}