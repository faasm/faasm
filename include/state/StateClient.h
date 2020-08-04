#pragma once

#include "State.h"
#include "InMemoryStateRegistry.h"

#include <grpcpp/client_context.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/channel_arguments.h>

#include <proto/faasm.pb.h>
#include <proto/faasm.grpc.pb.h>

using namespace grpc;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

#define CHECK_RPC(label, op)  Status __status = op; \
        if(!__status.ok()) {   \
            printf("RPC error %s: %s\n", std::string(label).c_str(),  __status.error_message().c_str());    \
            throw std::runtime_error("RPC error " + std::string(label));    \
        }


namespace state {
    ChannelArguments getChannelArgs();

    class StateClient {
    public:
        explicit StateClient(const std::string &userIn, const std::string &keyIn, const std::string &hostIn);

        const std::string user;
        const std::string key;
        const std::string host;

        InMemoryStateRegistry &reg;

        std::shared_ptr<Channel> channel;
        std::unique_ptr<message::StateRPCService::Stub> stub;

        void pushChunks(const std::vector<StateChunk> &chunks);

        void pullChunks(const std::vector<StateChunk> &chunks, uint8_t *bufferStart);

        void append(const uint8_t *data, size_t length);

        void pullAppended(uint8_t *buffer, size_t length, long nValues);

        void clearAppended();

        size_t stateSize();

        void deleteState();

        void lock();

        void unlock();
    };
}