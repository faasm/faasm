#include "StateClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>


namespace state {
    StateClient::StateClient(const std::string &hostIn) :
            stub(message::StateRPCService::NewStub(channel)),
            host(hostIn),
            reg(state::getInMemoryStateRegistry()),
            channel(grpc::CreateChannel(
                    host + ":" + std::to_string(STATE_PORT),
                    grpc::InsecureChannelCredentials()
            )) {
    }

    void StateClient::sendShutdownRequestToServer() {
        message::StateRequest request;
        message::StateResponse response;
        Status status = stub->Shutdown(getContext(), request, &response);
        if(!status.ok()) {
            throw std::runtime_error("Failed to send shutdown message");
        }
    }

    ClientContext *StateClient::getContext() {
        return &context;
    }
}