#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#define KV_FROM_REQUEST(request) auto kv = std::static_pointer_cast<InMemoryStateKeyValue>( \
    state.getKV(request->user(), request->key()) \
    );

namespace state {
    StateServer::StateServer(State &stateIn) :
            state(stateIn), host(STATE_HOST), port(STATE_PORT) {
    }

    void StateServer::doStart() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::string serverAddr = host + ":" + std::to_string(port);

        // Build the server
        EnableDefaultHealthCheckService(true);
        reflection::InitProtoReflectionServerBuilderPlugin();
        ServerBuilder builder;
        builder.SetMaxMessageSize(MAX_STATE_MESSAGE_SIZE);
        builder.AddListeningPort(serverAddr, InsecureServerCredentials());
        builder.RegisterService(this);

        // Start it
        server = builder.BuildAndStart();
        logger->info("State server listening on {}", serverAddr);

        _started = true;

        server->Wait();
    }

    void StateServer::start(bool background) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if(background) {
            logger->debug("Starting state server in background thread");
            // Run the serving thread in the background. This is necessary to
            // be able to kill it from the main thread.
            servingThread = std::thread([this] {
                doStart();
            });

            _isBackground = true;
        } else {
            logger->debug("Starting state server in this thread");
            doStart();
            _isBackground = false;
        }
    }

    void StateServer::stop() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        if (_started) {
            logger->info("State server stopping");
            server->Shutdown();

            if(_isBackground) {
                logger->debug("Waiting for state server background thread");
                if (servingThread.joinable()) {
                    servingThread.join();
                }
            }
        } else {
            logger->info("Not stopping state server, never started");
        }
    }

    Status StateServer::Pull(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Pull {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->buildStatePullResponse(response);

        return Status::OK;
    }

    Status StateServer::PullChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateChunkResponse *response) {
        util::getLogger()->debug("Pull chunk {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->buildStatePullChunkResponse(request, response);

        return Status::OK;
    }

    Status StateServer::Push(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Push {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->extractStatePushData(request, response);

        return Status::OK;
    }

    Status StateServer::Size(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateSizeResponse *response) {
        util::getLogger()->debug("Size {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->buildStateSizeResponse(response);

        return Status::OK;
    }

    Status StateServer::PushChunk(
            ServerContext *context,
            const message::StateChunkRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Push chunk {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->extractStatePushChunkData(request, response);

        return Status::OK;
    }

    Status StateServer::PushManyChunk(
            ServerContext *context,
            const message::StateManyChunkRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Push many chunk {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->extractStatePushMultiChunkData(request, response);

        return Status::OK;
    }

    Status StateServer::Append(
            ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Append {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        kv->extractStateAppendData(request, response);

        return Status::OK;
    }

    Status StateServer::ClearAppended(
            ServerContext *context,
            const ::message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Clear appended {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)

        kv->clearAppended();

        return Status::OK;
    }

    Status StateServer::PullAppended(
            grpc::ServerContext *context,
            const ::message::StateAppendedRequest *request,
            message::StateAppendedResponse *response) {
        util::getLogger()->debug("Pull appended {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)

        kv->buildPullAppendedResponse(request, response);

        return Status::OK;
    }

    Status StateServer::Lock(
            grpc::ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Lock {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)
        kv->lockWrite();

        return Status::OK;
    }

    Status StateServer::Unlock(
            grpc::ServerContext *context,
            const message::StateRequest *request,
            message::StateResponse *response) {
        util::getLogger()->debug("Unlock {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)
        kv->unlockWrite();

        return Status::OK;
    }

    Status StateServer::Delete(grpc::ServerContext *context, const message::StateRequest *request,
                               message::StateResponse *response) {

        util::getLogger()->debug("Delete {}/{}", request->user(), request->key());

        state.deleteKV(request->user(), request->key());


        return Status::OK;
    }
}
