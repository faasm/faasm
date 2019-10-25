#include "KnativeNativeHandler.h"
#include "KnativeNativeEndpoint.h"

#include <utility>

namespace knative_native {
    KnativeNativeEndpoint::KnativeNativeEndpoint(
            std::string userIn,
            std::string funcIn
    ) : Endpoint(8080,
                 2),
        user(std::move(userIn)),
        func(std::move(funcIn)) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeNativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeNativeHandler>(user, func);
    }
}
