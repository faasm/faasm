#include "pistache/endpoint.h"
#include <util/logging.h>
#include <signal.h>

using namespace Pistache;

class HelloHandler : public Http::Handler {
public:
    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request& request, Http::ResponseWriter response) override{
        response.send(Pistache::Http::Code::Ok, "Hello World\n");
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

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8080));
    auto opts = Pistache::Http::Endpoint::options()
            .threads(5);

    Http::Endpoint server(addr);
    server.init(opts);
    server.setHandler(Http::make_handler<HelloHandler>());
    server.serve();

    // Wait for a signal
    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0) {
        logger->info("Received signal: {}", signal);
    } else {
        logger->info("Sigwait return value: {}", signal);
    }
}
