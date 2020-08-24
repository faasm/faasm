#include "RPCServer.h"

#include <faabric/util/logging.h>
#include <grpcpp/grpcpp.h>

namespace faabric::rpc {
    RPCServer::RPCServer(const std::string &hostIn, int portIn) :
            host(hostIn), port(portIn) {
    }

    void RPCServer::start(bool background) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();
        std::string serverAddr = host + ":" + std::to_string(port);

        _started = true;
        _isBackground = background;

        if (background) {
            logger->debug("Starting state server in background thread");
            // Run the serving thread in the background. This is necessary to
            // be able to kill it from the main thread.
            servingThread = std::thread([this, serverAddr] {
                doStart(serverAddr);
            });

        } else {
            logger->debug("Starting state server in this thread");
            doStart(serverAddr);
        }
    }

    void RPCServer::stop() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();
        if (!_started) {
            logger->info("Not stopping state server, never started");
            return;
        }

        logger->info("State server stopping");
        server->Shutdown();

        if (_isBackground) {
            logger->debug("Waiting for state server background thread");
            if (servingThread.joinable()) {
                servingThread.join();
            }
        }
    }
}