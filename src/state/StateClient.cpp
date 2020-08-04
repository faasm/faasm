#include "StateClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>
#include <util/logging.h>


namespace state {
    ChannelArguments getChannelArgs() {
        ChannelArguments channelArgs;

        // Can set channel properties here if need be

        return channelArgs;
    }

    StateClient::StateClient(const std::string &hostIn) :
            host(hostIn),
            reg(state::getInMemoryStateRegistry()),
            channel(grpc::CreateCustomChannel(
                    host + ":" + std::to_string(STATE_PORT),
                    grpc::InsecureChannelCredentials(),
                    getChannelArgs()
            )),
            stub(message::StateRPCService::NewStub(channel)) {
    }

    void StateClient::pushChunks(
            const std::string &user,
            const std::string &key,
            const std::vector<StateChunk> &chunks
    ) {
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

    void StateClient::pullChunks(
            const std::string &user,
            const std::string &key,
            const std::vector<StateChunk> &chunks,
            uint8_t *bufferStart
    ) {
        ClientContext context;
        auto stream = stub->Pull(&context);

        // Writer thread in background
        std::thread writer([&user, &key, &chunks, &stream] {
            for (const auto &chunk : chunks) {
                message::StateChunkRequest request;
                request.set_user(user);
                request.set_key(key);
                request.set_offset(chunk.offset);
                request.set_chunksize(chunk.length);

                bool writeSuccess = stream->Write(request);
                if (!writeSuccess) {
                    util::getLogger()->error("Failed to request {}/{} ({} -> {})", user, key, chunk.offset,
                                             chunk.offset + chunk.length);
                    throw std::runtime_error("Failed to request pull chunk");
                }
            }

            stream->WritesDone();
        });

        // Reader
        message::StateChunk response;
        while (stream->Read(&response)) {
            // Set the chunk
            std::copy(response.data().begin(), response.data().end(), bufferStart + response.offset());
        }

        CHECK_RPC("pull_chunks", stream->Finish())
    }
}