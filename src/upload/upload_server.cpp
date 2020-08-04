#include <upload/UploadServer.h>

#include <util/logging.h>
#include <util/config.h>
#include <state/StateServer.h>

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
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