#pragma once

#include "StateKeyValue.h"

#include <cstdint>
#include <string>

#include <tcp/TCPMessage.h>


#define STATE_PORT 8003

namespace state {
    enum StateMessageType {
        ERROR_RESPONSE,
        OK_RESPONSE,
        STATE_APPEND,
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

    std::pair<std::string, std::string> getUserKeyFromStateMessage(tcp::TCPMessage *msg);

    tcp::TCPMessage *buildStateSizeRequest(const std::string &user, const std::string &key);

    tcp::TCPMessage *buildStateSizeResponse(const std::string &user, const std::string &key, size_t stateSize);

    size_t extractSizeResponse(const tcp::TCPMessage *msg);

    tcp::TCPMessage *buildStatePullRequest(const StateKeyValue *kv);

    tcp::TCPMessage *buildStatePullResponse(StateKeyValue *kv);

    void extractPullResponse(const tcp::TCPMessage *msg, StateKeyValue *kv);

    tcp::TCPMessage *buildStatePullChunkRequest(const StateKeyValue *kv, long offset, size_t length);

    tcp::TCPMessage *buildStatePullChunkResponse(tcp::TCPMessage *request, StateKeyValue *kv);

    void extractPullChunkResponse(const tcp::TCPMessage *msg, StateKeyValue *kv, long offset, size_t length);

    tcp::TCPMessage *buildStatePushRequest(StateKeyValue *kv);

    void extractStatePushData(const tcp::TCPMessage *msg, StateKeyValue *kv);

    tcp::TCPMessage *buildStatePushChunkRequest(StateKeyValue *kv, long offset, size_t length);

    void extractStatePushChunkData(const tcp::TCPMessage *msg, StateKeyValue *kv);

    tcp::TCPMessage *buildStatePushMultiChunkRequest(StateKeyValue *kv, const std::vector<StateChunk> &chunks);

    void extractStatePushMultiChunkData(const tcp::TCPMessage *msg, StateKeyValue *kv);

    tcp::TCPMessage *buildStateAppendRequest(StateKeyValue *kv, size_t length, const uint8_t *data);

    void extractStateAppendData(const tcp::TCPMessage *msg, StateKeyValue *kv);

    tcp::TCPMessage *buildPullAppendedRequest(StateKeyValue *kv, size_t length, long nValues);

    tcp::TCPMessage *buildPullAppendedResponse(tcp::TCPMessage *request, StateKeyValue *kv);

    void extractPullAppendedData(const tcp::TCPMessage *msg, uint8_t *buffer);

    tcp::TCPMessage *buildStateDeleteRequest(StateKeyValue *kv);

    tcp::TCPMessage *buildStateLockRequest(StateKeyValue *kv);

    tcp::TCPMessage *buildStateUnlockRequest(StateKeyValue *kv);
}