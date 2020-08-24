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
            stub(faabric::FunctionRPCService::NewStub(channel)) {
    }

    void FunctionCallClient::shareFunctionCall(const faabric::Message &call) {
        ClientContext context;
        faabric::FunctionStatusResponse response;
        CHECK_RPC("function_share", stub->ShareFunction(&context, call, &response));
    }

    void  FunctionCallClient::sendMPIMessage(const faabric::MPIMessage &msg) {
        ClientContext context;
        faabric::FunctionStatusResponse response;
        CHECK_RPC("mpi_message", stub->MPICall(&context, msg, &response));
    }
}