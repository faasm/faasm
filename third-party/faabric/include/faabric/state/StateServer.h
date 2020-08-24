#pragma once

#include "State.h"

#include <faabric/proto/RPCServer.h>
#include <proto/faabric.pb.h>
#include <proto/faabric.grpc.pb.h>

using namespace grpc;

namespace faabric::state {
class StateServer final : public rpc::RPCServer, public faabric::StateRPCService::Service {
    public:
        explicit StateServer(State &stateIn);

        Status Pull(
                ServerContext *context,
                ServerReaderWriter<faabric::StatePart, faabric::StateChunkRequest> *stream) override;

        Status Push(
                ServerContext *context,
                ServerReader<faabric::StatePart> *reader,
                faabric::StateResponse *response) override;

        Status Size(
                ServerContext *context,
                const faabric::StateRequest *request,
                faabric::StateSizeResponse *response) override;

        Status Append(
                ServerContext *context,
                const faabric::StateRequest *request,
                faabric::StateResponse *response) override;

        Status ClearAppended(
                ServerContext *context,
                const ::faabric::StateRequest *request,
                faabric::StateResponse *response) override;

        Status PullAppended(
                grpc::ServerContext *context,
                const ::faabric::StateAppendedRequest *request,
                faabric::StateAppendedResponse *response) override;

        Status Lock(
                grpc::ServerContext *context,
                const faabric::StateRequest *request,
                faabric::StateResponse *response) override;

        Status Unlock(
                grpc::ServerContext *context,
                const faabric::StateRequest *request,
                faabric::StateResponse *response) override;

        Status Delete(
                grpc::ServerContext *context,
                const faabric::StateRequest *request,
                faabric::StateResponse *response) override;
    protected:
        void doStart(const std::string &serverAddr) override;
    private:
        State &state;
    };
}