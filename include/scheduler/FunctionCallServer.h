#pragma once

#include "Scheduler.h"

#include <proto/RPCServer.h>
#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

namespace scheduler {
    class FunctionCallServer final : public rpc::RPCServer, public message::FunctionRPCService::Service {
    public:
        FunctionCallServer();

        Status ShareFunction(
                ServerContext *context,
                const message::Message *request,
                message::FunctionStatusResponse *response) override;

    protected:
        void doStart(const std::string &serverAddr) override;

    private:
        Scheduler &scheduler;
    };
}