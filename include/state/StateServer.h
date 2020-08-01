#pragma once

#include "State.h"
#include "StateMessage.h"

#include <grpcpp/server.h>
#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

namespace state {
    class StateServer final : public message::StateRPCService::Service {
    public:
        StateServer(State &stateIn, const std::string &hostIn, int portIn);

        void start();

        Status Pull(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status PullChunk(
                ServerContext *context,
                const message::StateChunkRequest *request,
                message::StateChunkResponse *response) override;

        Status Push(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status Size(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateSizeResponse *response) override;

        Status PushChunk(
                ServerContext *context,
                const message::StateChunkRequest *request,
                message::StateResponse *response) override;

        Status PushManyChunk(
                ServerContext *context,
                const message::StateManyChunkRequest *request,
                message::StateResponse *response) override;

        Status Append(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status Shutdown(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

    private:
        State &state;
        const std::string host;
        const int port;

        std::unique_ptr<Server> server;

        std::shared_ptr<StateKeyValue> getKv(const message::StateRequest *request);
    };
}