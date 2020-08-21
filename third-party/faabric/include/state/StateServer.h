#pragma once

#include "State.h"

#include <proto/RPCServer.h>
#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

namespace state {
class StateServer final : public rpc::RPCServer, public message::StateRPCService::Service {
    public:
        explicit StateServer(State &stateIn);

        Status Pull(
                ServerContext *context,
                ServerReaderWriter<message::StateChunk, message::StateChunkRequest> *stream) override;

        Status Push(
                ServerContext *context,
                ServerReader<message::StateChunk> *reader,
                message::StateResponse *response) override;

        Status Size(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateSizeResponse *response) override;

        Status Append(
                ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status ClearAppended(
                ServerContext *context,
                const ::message::StateRequest *request,
                message::StateResponse *response) override;

        Status PullAppended(
                grpc::ServerContext *context,
                const ::message::StateAppendedRequest *request,
                message::StateAppendedResponse *response) override;

        Status Lock(
                grpc::ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status Unlock(
                grpc::ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;

        Status Delete(
                grpc::ServerContext *context,
                const message::StateRequest *request,
                message::StateResponse *response) override;
    protected:
        void doStart(const std::string &serverAddr) override;
    private:
        State &state;
    };
}