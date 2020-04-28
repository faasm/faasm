#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>

namespace state {
    StateServer::StateServer() : tcp::TCPServer(STATE_PORT, util::getSystemConfig().globalMessageTimeout) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Key is the first bit of data, extract it
        uint8_t *bufferStart = recvMessage->buffer;
        auto keyStart = reinterpret_cast<char*>(bufferStart + sizeof(int32_t));
        int32_t keyLen = *(reinterpret_cast<int32_t *>(bufferStart));
        std::string stateKey(keyStart, keyStart + keyLen);

        // Get the key from state
        state::getGlobalState();

        // TODO - response

        // Handle the message accordingly
        switch (recvMessage->type) {
            case (StateMessageType::STATE_PULL): {
                logger->debug("State pull: {}", stateKey);
            }
            case (StateMessageType::STATE_PULL_CHUNK): {
                logger->debug("State pull chunk: {}", stateKey);
            }
            case (StateMessageType::STATE_PUSH): {
                logger->debug("State push: {}", stateKey);
            }
            case (StateMessageType::STATE_PUSH_CHUNK): {
                logger->debug("State push chunk {}", stateKey);
            }
            case (StateMessageType::STATE_LOCK): {
                logger->debug("State lock: {}", stateKey);
            }
            case (StateMessageType::STATE_UNLOCK): {
                logger->debug("State unlock: {}", stateKey);
            }
            case (StateMessageType::STATE_DELETE): {
                logger->debug("State delete: {}", stateKey);
            }
            default: {
                logger->error("Unrecognised request {}", recvMessage->type);
                throw std::runtime_error("Unrecognised state request type");
            }
        }
    }
}
