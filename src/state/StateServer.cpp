#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#define COUNT_MESSAGE if(messageLimit > 0) messageCount++;

#define KV_FROM_REQUEST(request) auto kv = std::static_pointer_cast<InMemoryStateKeyValue>( \
    state.getKV(request->user(), request->key()) \
    );

namespace state {
    StateServer::StateServer(State &stateIn) :
            StateServer(stateIn, 0) {

    }

    StateServer::StateServer(State &stateIn, int messageLimitIn) :
            state(stateIn), host(STATE_HOST), port(STATE_PORT),
            messageLimit(messageLimitIn) {

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

    void StateServer::stop() {
        util::getLogger()->info("State server stopping");
        server->Shutdown();
    }

    Status StateServer::Pull(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->buildStatePullResponse(response);

        return Status::OK;
    }

    Status StateServer::PullChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateChunkResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->buildStatePullChunkResponse(request, response);

        return Status::OK;
    }

    Status StateServer::Push(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->extractStatePushData(request, response);

        return Status::OK;
    }

    Status StateServer::Size(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateSizeResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->buildStateSizeResponse(response);

        return Status::OK;
    }

    Status StateServer::PushChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->extractStatePushChunkData(request, response);

        return Status::OK;
    }

    Status StateServer::PushManyChunk(
            ServerContext *context,
            const message::StateManyChunkRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->extractStatePushMultiChunkData(request, response);

        return Status::OK;
    }

    Status StateServer::Append(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->extractStateAppendData(request, response);

        return Status::OK;
    }

    Status StateServer::Shutdown(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        // TODO - can we have the server shut itself down like this?
        server->Shutdown();

        return Status::OK;
    }

    Status StateServer::ClearAppended(
            ServerContext *context,
            const ::message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)

        kv->clearAppended();
        return Status::OK;
    }

    Status StateServer::PullAppended(
            grpc::ServerContext *context,
            const ::message::StateAppendedRequest *request,
            message::StateAppendedResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)

        kv->buildPullAppendedResponse(request, response);

        return Status::OK;
    }

    Status StateServer::Lock(
            grpc::ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->lockWrite();
        return Status::OK;
    }

    Status StateServer::Unlock(
            grpc::ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        COUNT_MESSAGE;

        KV_FROM_REQUEST(request)
        kv->unlockWrite();
        return Status::OK;
    }

    Status StateServer::Delete(grpc::ServerContext *context, const message::StateRequest *request,
                               message::StateResponse *response) {
        COUNT_MESSAGE;

        state.deleteKV(request->user(), request->key());
        return Status::OK;
    }
}
