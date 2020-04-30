#include "StateMessage.h"

#include <util/logging.h>
#include <util/macros.h>

namespace state {

    size_t getTCPMessageDataOffset(const std::string &user, const std::string &key) {
        return sizeof(int32_t) + user.size() + sizeof(int32_t) + key.size();
    }

    std::pair<std::string, std::string> getUserKeyFromStateMessage(tcp::TCPMessage *msg) {
        // User/key come first in the message, each preceded by their length
        uint8_t *userLenStart = msg->buffer;
        int32_t userLen = *(reinterpret_cast<int32_t *>(userLenStart));

        uint8_t *userBufferStart = userLenStart + sizeof(int32_t);
        auto userBuffer = reinterpret_cast<char *>(userBufferStart);

        uint8_t *keyLenStart = userBufferStart + userLen;
        int32_t keyLen = *(reinterpret_cast<int32_t *>(keyLenStart));

        uint8_t *keyBufferStart = keyLenStart + sizeof(int32_t);
        auto keyBuffer = reinterpret_cast<char *>(keyBufferStart);

        std::string user(userBuffer, userBuffer + userLen);
        std::string key(keyBuffer, keyBuffer + keyLen);

        return {user, key};
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

    tcp::TCPMessage *buildStateSizeRequest(const std::string &user, const std::string &key) {
        tcp::TCPMessage *msg = buildStateTCPMessage(
                StateMessageType::STATE_SIZE,
                user,
                key,
                0);

        return msg;
    }

    tcp::TCPMessage *buildStateSizeResponse(const std::string &user, const std::string &key, size_t stateSize) {
        auto response = new tcp::TCPMessage();
        response->type = StateMessageType::STATE_SIZE_RESPONSE;
        response->len = sizeof(size_t);
        response->buffer = new uint8_t[sizeof(size_t)];

        std::copy(BYTES(&stateSize), BYTES(&stateSize) + sizeof(size_t), response->buffer);
        return response;
    }

    size_t extractSizeResponse(const tcp::TCPMessage *msg) {
        if (msg->type == StateMessageType::STATE_SIZE_RESPONSE) {
            size_t stateSize = *(reinterpret_cast<size_t *>(msg->buffer));
            return stateSize;
        } else {
            util::getLogger()->error("Unexpected response from size ({})", msg->type);
            throw std::runtime_error("Pull failed");
        }
    }

    void extractPullAppendedData(const tcp::TCPMessage *msg, uint8_t *buffer) {
        // Directly copy received data
        std::copy(msg->buffer, msg->buffer + msg->len, buffer);
    }
}