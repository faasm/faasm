#include "KnativeHandler.h"

using namespace Pistache;

namespace worker {
    void KnativeHandler::onRequest(const Http::Request &request, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Hello from C++ Faasm");
    }
}
