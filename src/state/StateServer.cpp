#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

namespace state {
    StateServer::StateServer(State &stateIn) : tcp::TCPServer(STATE_PORT, -1),
                                               state(stateIn) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *recvMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::pair<std::string, std::string> userKeyPair = getUserKeyFromStateMessage(recvMessage);
        const std::string &user = userKeyPair.first;
        const std::string &key = userKeyPair.second;

        // Get the size. State should be mastered on this host, hence we don't need
        // to specify size (will error if not the case).
        std::shared_ptr<StateKeyValue> localKv = state.getKV(user, key, 0);
        std::shared_ptr<InMemoryStateKeyValue> kv = std::static_pointer_cast<InMemoryStateKeyValue>(localKv);

        size_t stateSize = kv->size();

        // Construct appropriate response
        tcp::TCPMessage *response = nullptr;
        int requestType = recvMessage->type;
        if (requestType == StateMessageType::STATE_SIZE) {
            logger->debug("State size: {}", key);
            response = buildStateSizeResponse(user, key, stateSize);

        } else if (requestType == StateMessageType::STATE_PULL) {
            logger->debug("State pull: {}", key);
            response = kv->buildStatePullResponse();

        } else if (requestType == StateMessageType::STATE_PULL_CHUNK) {
            logger->debug("State pull chunk: {}", key);
            response = kv->buildStatePullChunkResponse(recvMessage);

        } else if (requestType == StateMessageType::STATE_PUSH) {
            logger->debug("State push: {}", key);
            kv->extractStatePushData(recvMessage);
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_PUSH_CHUNK) {
            logger->debug("State push chunk {}", key);
            kv->extractStatePushChunkData(recvMessage);
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_PUSH_MANY_CHUNK) {
            logger->debug("State push many chunk {}", key);
            kv->extractStatePushMultiChunkData(recvMessage);
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_APPEND) {
            logger->debug("State append {}", key);
            kv->extractStateAppendData(recvMessage);
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_CLEAR_APPENDED) {
            logger->debug("State clear appended {}", key);
            kv->clearAppended();
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_PULL_APPENDED) {
            logger->debug("State pull appended {}", key);
            response = kv->buildPullAppendedResponse(recvMessage);

        } else if (requestType == StateMessageType::STATE_LOCK) {
            logger->debug("State lock: {}", key);
            localKv->lockGlobal();
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_UNLOCK) {
            logger->debug("State unlock: {}", key);
            localKv->unlockGlobal();
            response = kv->buildOkResponse();

        } else if (requestType == StateMessageType::STATE_DELETE) {
            logger->debug("State delete: {}", key);
            response = kv->buildOkResponse();
            state.deleteKV(user, key);
        } else {
            logger->error("Unrecognised request {}", requestType);
            throw std::runtime_error("Unrecognised state request type");
        }

        return response;
    }
}
