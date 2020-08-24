#include <upload/UploadServer.h>

#include <faabric/util/logging.h>
#include <faabric/util/config.h>
#include <faabric/state/StateServer.h>

int main() {
    faabric::utilinitLogging();

    faabric::utilSystemConfig &config = faabric::utilgetSystemConfig();
    config.print();

    // Add a state server in the background
    state::StateServer stateServer(state::getGlobalState());
    stateServer.start();

    // Start the upload server in the main thread
    edge::UploadServer server;
    server.listen(UPLOAD_PORT);

    // Stop the state server
    stateServer.stop();

    return 0;
}