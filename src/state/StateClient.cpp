#include "StateClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>


namespace state {
    StateClient::StateClient(const std::string &hostIn) :
            host(hostIn),
            reg(state::getInMemoryStateRegistry()),
            channel(grpc::CreateChannel(
                    host + ":" + std::to_string(STATE_PORT),
                    grpc::InsecureChannelCredentials()
            )),
            stub(message::StateRPCService::NewStub(channel)) {
    }

    std::unique_ptr<ClientContext> StateClient::getContext() {
        return std::make_unique<ClientContext>();
    }
}