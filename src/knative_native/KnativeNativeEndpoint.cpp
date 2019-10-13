#include "KnativeNativeHandler.h"
#include "KnativeNativeEndpoint.h"

namespace knative_native {
    KnativeNativeEndpoint::KnativeNativeEndpoint() : Endpoint(8080, 2) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeNativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeNativeHandler>();
    }
}
