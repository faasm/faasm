#include "KnativeHandler.h"
#include "KnativeEndpoint.h"

namespace knative {
    KnativeEndpoint::KnativeEndpoint() : KnativeEndpoint(8080){ }

    KnativeEndpoint::KnativeEndpoint(int port) : Endpoint(port, 4) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeHandler>();
    }
}
