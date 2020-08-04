#include "StateClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>
#include <util/logging.h>
#include <util/macros.h>


namespace state {
    ChannelArguments getChannelArgs() {
        ChannelArguments channelArgs;

        // Can set channel properties here if need be

        return channelArgs;
    }

    StateClient::StateClient(
            const std::string &userIn,
            const std::string &keyIn,
            const std::string &hostIn
    ) :
            user(userIn),
            key(keyIn),
            host(hostIn),
            reg(state::getInMemoryStateRegistry()),
            channel(grpc::CreateCustomChannel(
                    host + ":" + std::to_string(STATE_PORT),
                    grpc::InsecureChannelCredentials(),
                    getChannelArgs()
            )),
            stub(message::StateRPCService::NewStub(channel)) {
    }

    void StateClient::pushChunks(const std::vector<StateChunk> &chunks) {
        message::StateResponse response;
        ClientContext clientContext;
        auto stream = stub->Push(&clientContext, &response);

        for (auto chunk: chunks) {
            message::StateChunk c;
            c.set_user(user);
            c.set_key(key);
            c.set_offset(chunk.offset);
            c.set_data(chunk.data, chunk.length);

            if (!stream->Write(c)) {
                break;
            };
        }

        stream->WritesDone();

        CHECK_RPC("push_chunks", stream->Finish());
    }

    void StateClient::pullChunks(const std::vector<StateChunk> &chunks, uint8_t *bufferStart) {
        ClientContext context;
        auto stream = stub->Pull(&context);

        // Writer thread in background
        std::thread writer([this, &chunks, &stream] {
            for (const auto &chunk : chunks) {
                message::StateChunkRequest request;
                request.set_user(user);
                request.set_key(key);
                request.set_offset(chunk.offset);
                request.set_chunksize(chunk.length);

                if (!stream->Write(request)) {
                    util::getLogger()->error("Failed to request {}/{} ({} -> {})", user, key, chunk.offset,
                                             chunk.offset + chunk.length);
                    break;
                }
            }

            stream->WritesDone();
        });

        // Read responses in this thread
        message::StateChunk response;
        while (stream->Read(&response)) {
            std::copy(response.data().begin(), response.data().end(), bufferStart + response.offset());
        }

        // Wait for the writer thread
        if (writer.joinable()) {
            writer.join();
        }

        CHECK_RPC("pull_chunks", stream->Finish())
    }

    void StateClient::append(const uint8_t *data, size_t length) {
        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);
        request.set_data(data, length);

        ClientContext context;
        CHECK_RPC("state_append", stub->Append(&context, request, &response))
    }

    void StateClient::pullAppended(uint8_t *buffer, size_t length, long nValues) {
        message::StateAppendedRequest request;
        message::StateAppendedResponse response;

        request.set_user(user);
        request.set_key(key);
        request.set_nvalues(nValues);

        ClientContext context;
        CHECK_RPC("state_pull_appended", stub->PullAppended(&context, request, &response))

        size_t offset = 0;
        for (auto &value : response.values()) {
            if (offset > length) {
                util::getLogger()->error("Buffer not large enough for appended data (offset={}, length={})", offset,
                                         length);
                throw std::runtime_error("Buffer not large enough for appended data");
            }

            auto valueData = BYTES_CONST(value.data().c_str());
            std::copy(valueData, valueData + value.data().size(), buffer + offset);
            offset += value.data().size();
        }
    }

    void StateClient::clearAppended() {
        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);

        ClientContext context;
        CHECK_RPC("state_clear_appended", stub->ClearAppended(&context, request, &response))
    }

    size_t StateClient::stateSize() {
        message::StateRequest request;
        request.set_user(user);
        request.set_key(key);

        message::StateSizeResponse response;
        ClientContext context;
        CHECK_RPC("state_size", stub->Size(&context, request, &response))
        return response.statesize();
    }

    void StateClient::deleteState() {
        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);

        ClientContext context;
        CHECK_RPC("state_delete", stub->Delete(&context, request, &response))
    }

    void StateClient::lock() {
        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);

        ClientContext context;
        CHECK_RPC("state_lock", stub->Lock(&context, request, &response))
    }

    void StateClient::unlock() {
        message::StateRequest request;
        message::StateResponse response;

        request.set_user(user);
        request.set_key(key);

        ClientContext context;
        CHECK_RPC("state_unlock", stub->Unlock(&context, request, &response))
    }
}