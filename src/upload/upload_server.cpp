#include <upload/UploadServer.h>

#include <faabric/state/StateServer.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>

int main()
{
    faabric::util::initLogging();

    faabric::util::SystemConfig& config = faabric::util::getSystemConfig();
    config.print();

    // Add a state server in the background
    faabric::state::StateServer stateServer(faabric::state::getGlobalState());
    stateServer.start();

    // Start the upload server in the main thread
    edge::UploadServer server;
    server.listen(UPLOAD_PORT);

    // Stop the state server
    stateServer.stop();

    return 0;
}
