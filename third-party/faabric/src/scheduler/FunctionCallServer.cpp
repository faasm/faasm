#include "FunctionCallServer.h"
#include "MpiWorldRegistry.h"

#include <faabric/util/logging.h>
#include <faabric/util/config.h>

#include <grpcpp/grpcpp.h>
#include <faabric/proto/macros.h>


namespace faabric::scheduler {
    FunctionCallServer::FunctionCallServer() :
            RPCServer(DEFAULT_RPC_HOST, FUNCTION_CALL_PORT),
            scheduler(getScheduler()) {

    }

    void FunctionCallServer::doStart(const std::string &serverAddr) {
        // Build the server
        ServerBuilder builder;
        builder.AddListeningPort(serverAddr, InsecureServerCredentials());
        builder.RegisterService(this);

        // Start it
        server = builder.BuildAndStart();
        faabric::utilgetLogger()->info("Function call server listening on {}", serverAddr);

        server->Wait();
    }

    Status FunctionCallServer::ShareFunction(
            ServerContext *context,
            const faabric::Message *request,
            faabric::FunctionStatusResponse *response) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();

        // TODO - avoiding having to copy the message here
        faabric::Message msg = *request;

        // This calls the scheduler, which will always attempt
        // to execute locally. However, if not possible, this will
        // again share the message, increasing the hops
        const std::string funcStr = faabric::utilfuncToString(msg, true);
        logger->debug("{} received shared call {} (scheduled for {})", host, funcStr,
                      msg.scheduledhost());

        scheduler.callFunction(msg);

        return Status::OK;
    }

    Status FunctionCallServer::MPICall(
            ServerContext *context,
            const faabric::MPIMessage *request,
            faabric::FunctionStatusResponse *response) {

        // TODO - avoid copying message
        faabric::MPIMessage m = *request;

        MpiWorldRegistry &registry = getMpiWorldRegistry();
        MpiWorld &world = registry.getWorld(m.worldid());
        world.enqueueMessage(m);

        return Status::OK;
    }
}