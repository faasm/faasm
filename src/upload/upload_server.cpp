#include <upload/UploadServer.h>


#include <util/logging.h>
#include <util/config.h>
#include <state/StateServer.h>

static std::atomic<bool> isShutdown = false;

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    // Add a state server in the background
    std::thread stateThread([] {
        state::StateServer server(state::getGlobalState());
        while (!isShutdown.load()) {
            server.poll();
        }

        server.close();
    });

    // Start the upload server in the main thread
    edge::UploadServer server;
    server.listen(UPLOAD_PORT);

    // Flag the state server to shut down
    isShutdown.store(true);

    // Wait for state server to shut down
    if(stateThread.joinable()) {
        stateThread.join();
    }

    return 0;
}