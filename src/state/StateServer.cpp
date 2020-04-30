#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>

namespace state {
    StateServer::StateServer() : tcp::TCPServer(STATE_PORT, util::getSystemConfig().globalMessageTimeout) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::pair<std::string, std::string> userKeyPair = getUserKeyFromStateMessage(recvMessage);
        const std::string &user = userKeyPair.first;
        const std::string &key = userKeyPair.second;

        // Get the size. State should be mastered on this host, hence we don't need
        // to specify size (will error if not the case).
        State &state = state::getGlobalState();
        const std::shared_ptr<StateKeyValue> &kv = state.getKV(user, key, 0);
        size_t stateSize = kv->size();

        // Construct appropriate response
        tcp::TCPMessage *response = nullptr;
        int requestType = recvMessage->type;
        if (requestType == StateMessageType::STATE_SIZE) {
            logger->debug("State size: {}", key);
            response = buildStateSizeResponse(user, key, stateSize);

        } else if (requestType == StateMessageType::STATE_PULL) {
            logger->debug("State pull: {}", key);
            response = buildStatePullResponse(kv.get());

        } else if (requestType == StateMessageType::STATE_PULL_CHUNK) {
            logger->debug("State pull chunk: {}", key);
            response = buildStatePullChunkResponse(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_PUSH) {
            logger->debug("State push: {}", key);
            extractStatePushData(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_PUSH_CHUNK) {
            logger->debug("State push chunk {}", key);
            extractStatePushChunkData(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_PUSH_MANY_CHUNK) {
            logger->debug("State push many chunk {}", key);
            extractStatePushMultiChunkData(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_APPEND) {
            logger->debug("State append {}", key);
            extractStateAppendData(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_PULL_APPENDED) {
            logger->debug("State pull appended {}", key);
            response = buildPullAppendedResponse(recvMessage, kv.get());

        } else if (requestType == StateMessageType::STATE_LOCK) {
            logger->debug("State lock: {}", key);
            kv->lockGlobal();

        } else if (requestType == StateMessageType::STATE_UNLOCK) {
            logger->debug("State unlock: {}", key);
            kv->unlockGlobal();

        } else if (requestType == StateMessageType::STATE_DELETE) {
            logger->debug("State delete: {}", key);
            kv->deleteGlobal();

        } else {
            logger->error("Unrecognised request {}", requestType);
            throw std::runtime_error("Unrecognised state request type");
        }

        return response;
    }
}
