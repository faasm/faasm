#include "KnativeHandler.h"
#include "KnativeEndpoint.h"

namespace knative {
    KnativeEndpoint::KnativeEndpoint() : HttpEndpoint(8080, 4) {
    }

    void KnativeEndpoint::setHandler() {
        httpEndpoint->setHandler(std::make_shared<KnativeHandler>());
    }
}
