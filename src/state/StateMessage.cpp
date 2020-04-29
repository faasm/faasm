#include "StateMessage.h"

#include <util/logging.h>
#include <util/macros.h>

namespace state {

    size_t getTCPMessageDataOffset(const std::string &user, const std::string &key) {
        return sizeof(int32_t) + user.size() + sizeof(int32_t) + key.size();
    }

    tcp::TCPMessage *buildStateTCPMessage(int msgType,
                                          const std::string &user, const std::string &key,
                                          size_t dataSize) {

        // Get sizes
        auto userSize = (int32_t) user.size();
        auto keySize = (int32_t) key.size();
        size_t fullSize = sizeof(int32_t) + userSize + sizeof(int32_t) + keySize + dataSize;

        auto msg = new tcp::TCPMessage();
        msg->type = msgType;
        msg->len = fullSize;
        msg->buffer = new uint8_t[fullSize];

        // Fill buffer with: user size | user | key size | key
        auto userBytes = BYTES_CONST(user.data());
        std::copy(BYTES(&userSize), BYTES(&userSize) + sizeof(int32_t), msg->buffer);
        std::copy(userBytes, userBytes + userSize, msg->buffer + sizeof(int32_t));

        size_t keyOffset = userSize + sizeof(int32_t);
        auto keyBytes = BYTES_CONST(key.data());
        std::copy(BYTES(&keySize), BYTES(&keySize) + sizeof(int32_t), msg->buffer + keyOffset);
        std::copy(keyBytes, keyBytes + keySize, msg->buffer + keyOffset + sizeof(int32_t));

        return msg;
    }

    tcp::TCPMessage *buildStateSizeMessage(const std::string &user, const std::string &key) {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_SIZE,
                user,
                key,
                0);

        return msg;
    }

    size_t extractSizeResponse(const tcp::TCPMessage *msg) {
        if (msg->type == StateMessageType::STATE_SIZE_RESPONSE) {
            size_t stateSize = *(reinterpret_cast<size_t *>(msg->buffer));
            return stateSize;
        } else {
            util::getLogger()->error("Unexpected response from pull ({})", msg->type);
            throw std::runtime_error("Pull failed");
        }
    }

    tcp::TCPMessage *buildStatePullMessage(const StateKeyValue *kv) {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_PULL,
                kv->user,
                kv->key,
                0);
        return msg;
    }

    void extractPullResponse(const tcp::TCPMessage *msg, StateKeyValue *kv) {
        if (msg->type == StateMessageType::STATE_PULL_RESPONSE) {
            kv->set(msg->buffer);
        } else {
            util::getLogger()->error("Unexpected response from pull ({})", msg->type);
            throw std::runtime_error("Pull failed");
        }
    }

    tcp::TCPMessage *buildStatePullChunkMessage(const StateKeyValue *kv, long offset, size_t length) {
        std::string user = kv->user;
        std::string key = kv->key;

        // Buffer contains two ints, offset and length
        size_t dataSize = 2 * sizeof(int32_t);
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PULL_CHUNK, user, key, dataSize);

        // Copy offset and length into buffer
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto offSetInt = (int32_t) offset;
        auto lengthInt = (int32_t) length;
        std::copy(BYTES(&offSetInt), BYTES(&offSetInt) + sizeof(int32_t), msg->buffer + dataOffset);
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), msg->buffer + dataOffset + sizeof(int32_t));

        return msg;
    }

    tcp::TCPMessage *buildStatePullChunkResponse(tcp::TCPMessage *request, StateKeyValue *kv) {
        unsigned long dataOffset = getTCPMessageDataOffset(kv->user, kv->key);

        // Extract offset and length from request
        uint8_t *requestData = request->buffer + dataOffset;
        int32_t chunkOffset = *(reinterpret_cast<int32_t *>(requestData));
        int32_t chunkLen = *(reinterpret_cast<int32_t *>(requestData + sizeof(int32_t)));

        tcp::TCPMessage *response = new tcp::TCPMessage();
        response->type = StateMessageType::STATE_PULL_CHUNK_RESPONSE;
        response->len = chunkLen;

        // TODO - can we do this without copying?
        uint8_t *chunkPtr = kv->getSegment(chunkOffset, chunkLen);
        response->buffer = new uint8_t[chunkLen];

        kv->lockRead();
        std::copy(chunkPtr, chunkPtr + chunkLen, response->buffer);
        kv->unlockRead();

        return response;
    }
    
    void extractPullChunkResponse(const tcp::TCPMessage *msg, StateKeyValue *kv, long offset, size_t length) {
        if (msg->type == StateMessageType::STATE_PULL_CHUNK_RESPONSE) {
            // Set the chunk with the response data
            kv->setSegment(offset, msg->buffer, length);
        } else {
            util::getLogger()->error("Unexpected response from pull {}", msg->type);
            throw std::runtime_error("Pull range failed");
        }
    }

    tcp::TCPMessage *buildStatePushMessage(StateKeyValue *kv) {
        std::string user = kv->user;
        std::string key = kv->key;

        // Buffer contains length followed by the data
        size_t valueSize = kv->size();
        auto valuePtr = kv->get();
        size_t dataSize = sizeof(int32_t) + valueSize;
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PUSH, user, key, dataSize);

        // Copy length and data into buffer
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto sizeInt = (int32_t) valueSize;
        std::copy(BYTES(&sizeInt), BYTES(&sizeInt) + sizeof(int32_t), msg->buffer + dataOffset);

        // TODO - avoid copy here?
        kv->lockRead();
        std::copy(valuePtr, valuePtr + valueSize, msg->buffer + dataOffset + sizeof(int32_t));
        kv->unlockRead();

        return msg;
    }

    void extractPushData(const tcp::TCPMessage *msg, StateKeyValue *kv) {
        // Extract data from request (data is preceded by its length)
        size_t dataOffset = getTCPMessageDataOffset(kv->user, kv->key);
        uint8_t *data = msg->buffer + dataOffset + sizeof(int32_t);

        // Note - set() already performs necessary locking
        kv->set(data);
    }

    tcp::TCPMessage *buildStatePushChunkMessage(StateKeyValue *kv, long offset, size_t length) {
        std::string user = kv->user;
        std::string key = kv->key;

        // Buffer contains offset, length then the data
        size_t dataSize = (2 * sizeof(int32_t)) + length;
        tcp::TCPMessage *msg = buildStateTCPMessage(StateMessageType::STATE_PUSH_CHUNK, user, key, dataSize);

        // Write the offset, length and data
        unsigned long dataOffset = getTCPMessageDataOffset(user, key);
        auto offSetInt = (int32_t) offset;
        auto lengthInt = (int32_t) length;
        std::copy(BYTES(&offSetInt), BYTES(&offSetInt) + sizeof(int32_t), msg->buffer + dataOffset);
        std::copy(BYTES(&lengthInt), BYTES(&lengthInt) + sizeof(int32_t), msg->buffer + dataOffset + sizeof(int32_t));

        // TODO - avoid copy here?
        uint8_t *dataPtr = kv->getSegment(offset, length);
        kv->lockRead();
        std::copy(dataPtr, dataPtr + length, msg->buffer + dataOffset + (2 * sizeof(int32_t)));
        kv->unlockRead();

        return msg;
    }

    tcp::TCPMessage *buildStateDeleteMessage(StateKeyValue *kv) {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_DELETE,
                kv->user,
                kv->key,
                0
        );
        return msg;
    }
}