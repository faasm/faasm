#include <util/macros.h>
#include "StateMessage.h"


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
}