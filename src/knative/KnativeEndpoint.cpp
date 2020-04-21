#include "KnativeHandler.h"
#include "KnativeEndpoint.h"

namespace knative {

    KnativeEndpoint::KnativeEndpoint() : Endpoint() {
    }

    KnativeEndpoint::KnativeEndpoint(int port, int threadCount) : Endpoint(port, threadCount) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeHandler>();
    }

}
