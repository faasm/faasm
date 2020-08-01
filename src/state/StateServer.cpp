#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

namespace state {
    StateServer::StateServer(State &stateIn, const std::string &hostIn, int portIn) :
            state(stateIn), host(hostIn), port(portIn) {

    }

    void StateServer::start() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::string serverAddr = host + ":" + std::to_string(port);

        // Build the server
        EnableDefaultHealthCheckService(true);
        reflection::InitProtoReflectionServerBuilderPlugin();
        ServerBuilder builder;
        builder.AddListeningPort(serverAddr, InsecureServerCredentials());
        builder.RegisterService(this);

        // Start it
        server = builder.BuildAndStart();
        logger->info("State server listening on {}", serverAddr);

        server->Wait();
    }

    std::shared_ptr<StateKeyValue> StateServer::getKv(const message::StateRequest *request) {
        // This state should be mastered on this host, hence we don't need
        // to specify size (will error if not the case).
        std::shared_ptr<StateKeyValue> localKv = state.getKV(
                request->user(),
                request->key(),
                0
        );

        return localKv;
    }

    Status StateServer::Pull(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {

        auto kv = std::static_pointer_cast<InMemoryStateKeyValue>(getKv(request));
        kv->buildStatePullResponse(response);

        return Status::OK;
    }

    Status StateServer::PullChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateChunkResponse *response) {

    }

    Status StateServer::Push(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {

    }

    Status StateServer::Size(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateSizeResponse *response) {

        auto kv = std::static_pointer_cast<InMemoryStateKeyValue>(getKv(request));
        kv->buildStateSizeResponse(response);

        return Status::OK;
    }

    Status StateServer::PushChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateResponse *response) {

    }

    Status StateServer::PushManyChunk(
            ServerContext *context,
            const message::StateManyChunkRequest *request,
            message::StateResponse *response) {

    }

    Status StateServer::Append(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {

    }

    Status StateServer::Shutdown(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        // TODO - can we have the server shut itself down like this?
        server->Shutdown();

        return Status::OK;
    }

    tcp::TCPMessage *StateServer::handleMessage(tcp::TCPMessage *request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

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
