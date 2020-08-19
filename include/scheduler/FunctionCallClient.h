#pragma once

#include <grpcpp/client_context.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/channel_arguments.h>

#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace scheduler {
    class FunctionCallClient {
    public:
        explicit FunctionCallClient(const std::string &hostIn);

        const std::string host;

        std::shared_ptr<Channel> channel;
        std::unique_ptr<message::FunctionRPCService::Stub> stub;

        void shareFunctionCall(const message::Message &call);

        void sendMPIMessage(const message::MPIMessage &msg);
    };
}