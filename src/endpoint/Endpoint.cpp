#include "Endpoint.h"

#include <faabric/util/logging.h>
#include <pistache/listener.h>
#include <pistache/endpoint.h>
#include <signal.h>


namespace endpoint {
    Endpoint::Endpoint(int portIn, int threadCountIn): port(portIn), threadCount(threadCountIn) {
    }

    void Endpoint::start() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

        logger->info("Starting HTTP endpoint");

        // Set up signal handler
        sigset_t signals;
        if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGKILL) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || sigaddset(&signals, SIGQUIT) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {

            throw std::runtime_error("Install signal handler failed");
        }

        Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(this->port));

        // Configure endpoint
        auto opts = Pistache::Http::Endpoint::options()
                .threads(threadCount)
                .backlog(256)
                .flags(Pistache::Tcp::Options::ReuseAddr);

        Pistache::Http::Endpoint httpEndpoint(addr);
        httpEndpoint.init(opts);

        // Configure and start endpoint
        httpEndpoint.setHandler(this->getHandler());
        httpEndpoint.serveThreaded();

        // Wait for a signal
        logger->info("Awaiting signal");
        int signal = 0;
        int status = sigwait(&signals, &signal);
        if (status == 0) {
            logger->info("Received signal: {}", signal);
        } else {
            logger->info("Sigwait return value: {}", signal);
        }

        httpEndpoint.shutdown();
    }

}