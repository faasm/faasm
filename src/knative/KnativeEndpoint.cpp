#include "KnativeHandler.h"
#include "KnativeEndpoint.h"

namespace knative {
    KnativeEndpoint::KnativeEndpoint() : Endpoint(8080, 8) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeHandler>();
    }
}
