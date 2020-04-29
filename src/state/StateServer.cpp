#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <util/macros.h>

namespace state {
    StateServer::StateServer() : tcp::TCPServer(STATE_PORT, util::getSystemConfig().globalMessageTimeout) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // User/key is the first bit of data, extract it
        uint8_t *userLenStart = recvMessage->buffer;
        int32_t userLen = *(reinterpret_cast<int32_t *>(userLenStart));

        uint8_t *userBufferStart = userLenStart + sizeof(int32_t);
        auto userBuffer = reinterpret_cast<char *>(userBufferStart);

        uint8_t *keyLenStart = userBufferStart + userLen;
        int32_t keyLen = *(reinterpret_cast<int32_t *>(keyLenStart));

        uint8_t *keyBufferStart = keyLenStart + sizeof(int32_t);
        auto keyBuffer = reinterpret_cast<char *>(keyBufferStart);

        std::string user(userBuffer, userBuffer + userLen);
        std::string key(keyBuffer, keyBuffer + keyLen);

        // Get the size. State should be mastered on this host, hence we don't need
        // to specify size (will error if not the case).
        State &state = state::getGlobalState();
        const std::shared_ptr<StateKeyValue> &kv = state.getKV(user, key, 0);
        size_t stateSize = kv->size();
        uint8_t *statePtr = kv->get();

        // Construct appropriate response
        tcp::TCPMessage *response = nullptr;
        int requestType = recvMessage->type;
        if (requestType == StateMessageType::STATE_SIZE) {
            logger->debug("State size: {}", key);

            response = new tcp::TCPMessage();
            response->type = StateMessageType::STATE_SIZE_RESPONSE;
            response->len = sizeof(size_t);
            response->buffer = new uint8_t[sizeof(size_t)];
            std::copy(BYTES(&stateSize), BYTES(&stateSize) + sizeof(size_t), response->buffer);
        } else if (requestType == StateMessageType::STATE_PULL) {
            logger->debug("State pull: {}", key);

            response = new tcp::TCPMessage();
            response->type = StateMessageType::STATE_PULL_RESPONSE;
            response->len = stateSize;

            // TODO - can we do this without copying?
            response->buffer = new uint8_t[stateSize];

            kv->lockRead();
            std::copy(statePtr, statePtr + stateSize, response->buffer);
            kv->unlockRead();
        } else if (requestType == StateMessageType::STATE_PULL_CHUNK) {
            logger->debug("State pull chunk: {}", key);

            // Extract offset and length from request
            int32_t chunkOffset = *(reinterpret_cast<int32_t *>(keyBufferStart + keyLen));
            int32_t chunkLen = *(reinterpret_cast<int32_t *>(keyBufferStart + keyLen + sizeof(int32_t)));

            response = new tcp::TCPMessage();
            response->type = StateMessageType::STATE_PULL_CHUNK_RESPONSE;
            response->len = chunkLen;

            // TODO - can we do this without copying?
            uint8_t *chunkPtr = kv->getSegment(chunkOffset, chunkLen);
            response->buffer = new uint8_t[chunkLen];

            kv->lockRead();
            std::copy(chunkPtr, chunkPtr + chunkLen, response->buffer);
            kv->unlockRead();
        } else if (requestType == StateMessageType::STATE_PUSH) {
            logger->debug("State push: {}", key);

            // Extract data from request (data is preceded by its length)
            uint8_t *data = keyBufferStart + keyLen + sizeof(int32_t);

            kv->lockWrite();
            kv->set(data);
            kv->unlockWrite();
        } else if (requestType == StateMessageType::STATE_PUSH_CHUNK) {
            logger->debug("State push chunk {}", key);
        } else if (requestType == StateMessageType::STATE_LOCK) {
            logger->debug("State lock: {}", key);
        } else if (requestType == StateMessageType::STATE_UNLOCK) {
            logger->debug("State unlock: {}", key);
        } else if (requestType == StateMessageType::STATE_DELETE) {
            logger->debug("State delete: {}", key);
        } else {
            logger->error("Unrecognised request {}", requestType);
            throw std::runtime_error("Unrecognised state request type");
        }

        return response;
    }
}
