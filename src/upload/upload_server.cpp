#include <storage/S3Wrapper.h>
#include <upload/UploadServer.h>

#include <faabric/state/StateServer.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>

int main()
{
    storage::initSDK();
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    faabric::util::SystemConfig& config = faabric::util::getSystemConfig();
    config.print();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    {
        faabric::state::StateServer stateServer(
          faabric::state::getGlobalState());
        stateServer.start();

        // Start the upload server in the main thread
        edge::UploadServer server;
        server.listen(UPLOAD_PORT);

        // Stop the state server
        stateServer.stop();
    }

    faabric::transport::closeGlobalMessageContext();
    storage::cleanUpSDK();

    return 0;
}
