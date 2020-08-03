#include "StateClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>


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

    std::unique_ptr<ClientContext> StateClient::getContext() {
        return std::make_unique<ClientContext>();
    }
}