#include "FunctionCallServer.h"

#include <util/logging.h>
#include <util/config.h>

#include <grpcpp/grpcpp.h>
#include <proto/macros.h>


namespace scheduler {
    FunctionCallServer::FunctionCallServer() : RPCServer(DEFAULT_RPC_HOST, FUNCTION_CALL_PORT) {

    }

    void FunctionCallServer::doStart(const std::string &serverAddr) {
        // Build the server
        ServerBuilder builder;
        builder.AddListeningPort(serverAddr, InsecureServerCredentials());
        builder.RegisterService(this);

        // Start it
        server = builder.BuildAndStart();
        util::getLogger()->info("State server listening on {}", serverAddr);

        server->Wait();
    }

    Status FunctionCallServer::ShareFunction(
            ServerContext *context,
            const message::Message *request,
            message::FunctionStatusResponse *response) {



    }
}