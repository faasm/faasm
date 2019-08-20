#include "pistache/endpoint.h"
#include <util/logging.h>
#include <signal.h>
#include <endpoint/Endpoint.h>

using namespace Pistache;


class HelloHandler : public Http::Handler {
public:
HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request &request, Http::ResponseWriter response) override {
        response.send(Pistache::Http::Code::Ok, "Hello World\n");
    }
};

class HelloEndpoint : public endpoint::Endpoint {
public:
    HelloEndpoint(int port, int threads): endpoint::Endpoint(port, threads) {

    }

    std::shared_ptr<Pistache::Http::Handler> getHandler() {
        return Pistache::Http::make_handler<HelloHandler>();
    }
};

int main() {
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

    HelloEndpoint server(8080, 5);
    server.start();

    // Wait for a signal
    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0) {
        logger->info("Received signal: {}", signal);
    } else {
        logger->info("Sigwait return value: {}", signal);
    }
}
