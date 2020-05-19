#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

namespace state {
    StateServer::StateServer(State &stateIn) : tcp::TCPServer(STATE_PORT, -1),
                                               state(stateIn) {

    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check for shutdowm
        int requestType = request->type;
        if (requestType == StateMessageType::SHUTDOWN) {
            logger->debug("State server shutdown request received");
            throw tcp::TCPShutdownException("Received shutdown request");
        }

        std::pair<std::string, std::string> userKeyPair = getUserKeyFromStateMessage(request);
        const std::string &user = userKeyPair.first;
        const std::string &key = userKeyPair.second;

        // Get the size. State should be mastered on this host, hence we don't need
        // to specify size (will error if not the case).
        std::shared_ptr<StateKeyValue> localKv = state.getKV(user, key, 0);
        std::shared_ptr<InMemoryStateKeyValue> kv = std::static_pointer_cast<InMemoryStateKeyValue>(localKv);

        size_t stateSize = kv->size();

        // Construct appropriate response
        tcp::TCPMessage *response = nullptr;
        if (requestType == StateMessageType::STATE_SIZE) {
            response = buildStateSizeResponse(user, key, stateSize);
            logger->debug("State size: {} (size {})", key, stateSize);

        } else if (requestType == StateMessageType::STATE_PULL) {
            response = kv->buildStatePullResponse();
            logger->debug("State pull: {} (buffer len {})", key, response->len);

        } else if (requestType == StateMessageType::STATE_PULL_CHUNK) {
            response = kv->buildStatePullChunkResponse(request);
            logger->debug("State pull chunk: {} (buffer len {})", key, response->len);

        } else if (requestType == StateMessageType::STATE_PUSH) {
            kv->extractStatePushData(request);
            response = kv->buildOkResponse();
            logger->debug("State push: {} OK", key);

        } else if (requestType == StateMessageType::STATE_PUSH_CHUNK) {
            kv->extractStatePushChunkData(request);
            response = kv->buildOkResponse();
            logger->debug("State push chunk {} OK", key);

        } else if (requestType == StateMessageType::STATE_PUSH_MANY_CHUNK) {
            kv->extractStatePushMultiChunkData(request);
            response = kv->buildOkResponse();
            logger->debug("State push many chunk {} OK", key);

        } else if (requestType == StateMessageType::STATE_APPEND) {
            kv->extractStateAppendData(request);
            response = kv->buildOkResponse();
            logger->debug("State append {} OK", key);

        } else if (requestType == StateMessageType::STATE_CLEAR_APPENDED) {
            kv->clearAppended();
            response = kv->buildOkResponse();
            logger->debug("State clear appended {} OK", key);

        } else if (requestType == StateMessageType::STATE_PULL_APPENDED) {
            response = kv->buildPullAppendedResponse(request);
            logger->debug("State pull appended {} (buffer len {})", key, response->len);

        } else if (requestType == StateMessageType::STATE_LOCK) {
            localKv->lockGlobal();
            response = kv->buildOkResponse();
            logger->debug("State lock: {} OK", key);

        } else if (requestType == StateMessageType::STATE_UNLOCK) {
            localKv->unlockGlobal();
            response = kv->buildOkResponse();
            logger->debug("State unlock: {} OK", key);

        } else if (requestType == StateMessageType::STATE_DELETE) {
            state.deleteKV(user, key);
            response = kv->buildOkResponse();
            logger->debug("State delete: {} OK", key);
        } else {
            logger->error("Unrecognised request {}", requestType);
            throw std::runtime_error("Unrecognised state request type");
        }

        return response;
    }
}
