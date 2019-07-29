#include "KnativeHandler.h"
#include "KnativeEndpoint.h"

namespace worker {
    KnativeEndpoint::KnativeEndpoint() : HttpEndpoint(8080, 4) {

    }

    void KnativeEndpoint::setHandler() {
        httpEndpoint->setHandler(std::make_shared<KnativeHandler>());
    }
}
