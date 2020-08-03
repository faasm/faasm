#pragma once

#include "State.h"
#include "InMemoryStateRegistry.h"

#include <grpcpp/client_context.h>
#include <grpcpp/channel.h>

#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace state {
    class StateClient {
    public:
        explicit StateClient(const std::string &hostIn);

        const std::string host;
        InMemoryStateRegistry &reg;
        std::shared_ptr<Channel> channel;
        std::unique_ptr<message::StateRPCService::Stub> stub;

        std::unique_ptr<ClientContext> getContext();
    };
}