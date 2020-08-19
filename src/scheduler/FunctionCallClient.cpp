#include "FunctionCallClient.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>

#include <proto/macros.h>


namespace scheduler {
    FunctionCallClient::FunctionCallClient(
            const std::string &hostIn
    ) :
            host(hostIn),
            channel(grpc::CreateChannel(
                    host + ":" + std::to_string(FUNCTION_CALL_PORT),
                    grpc::InsecureChannelCredentials()
            )),
            stub(message::FunctionRPCService::NewStub(channel)) {
    }

    void FunctionCallClient::shareFunctionCall(const message::Message &call) {
        ClientContext context;
        message::FunctionStatusResponse response;
        CHECK_RPC("function_share", stub->ShareFunction(&context, call, &response));
    }
}