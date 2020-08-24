#pragma once

#include "Scheduler.h"

#include <proto/RPCServer.h>
#include <proto/faabric.pb.h>
#include <proto/faabric.grpc.pb.h>

using namespace grpc;

namespace scheduler {
    class FunctionCallServer final : public rpc::RPCServer, public faabric::FunctionRPCService::Service {
    public:
        FunctionCallServer();

        Status ShareFunction(
                ServerContext *context,
                const faabric::Message *request,
                faabric::FunctionStatusResponse *response) override;

        Status MPICall(
                ServerContext *context,
                const faabric::MPIMessage *request,
                faabric::FunctionStatusResponse *response) override;

    protected:
        void doStart(const std::string &serverAddr) override;

    private:
        Scheduler &scheduler;
    };
}