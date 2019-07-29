#include "KnativeHandler.h"

#include <util/logging.h>

using namespace Pistache;

namespace worker {
    void KnativeHandler::onRequest(const Http::Request &request, Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Received request");

        response.send(Http::Code::Ok, "Hello from C++ Faasm");
    }
}
