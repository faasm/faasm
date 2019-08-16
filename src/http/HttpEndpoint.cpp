#include "HttpEndpoint.h"

#include <util/logging.h>
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <signal.h>


namespace http {
    HttpEndpoint::HttpEndpoint(int port, int threadCount) {
        Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(port));

        // Configure endpoint
        auto opts = Pistache::Http::Endpoint::options()
                .threads(threadCount)
                .flags(Pistache::Tcp::Options::ReuseAddr);

        httpEndpoint = std::make_shared<Pistache::Http::Endpoint>(addr);
        httpEndpoint->init(opts);
    }

    void HttpEndpoint::start() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up signal handler
        sigset_t signals;
        if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {

            throw std::runtime_error("Install signal handler failed");
        }

        // Configure and start endpoint
        this->setHandler();
        httpEndpoint->serve();

        // Wait for a signal
        int signal = 0;
        int status = sigwait(&signals, &signal);
        if (status == 0) {
            logger->info("Received signal: {}", signal);
        } else {
            logger->info("Sigwait return value: {}", signal);
        }

        httpEndpoint->shutdown();
    }

}