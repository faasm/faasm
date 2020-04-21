#include "KnativeNativeHandler.h"
#include "KnativeNativeEndpoint.h"

#include <utility>

namespace knative_native {

    KnativeNativeEndpoint::KnativeNativeEndpoint(std::string userIn, std::string funcIn) : Endpoint(),
                                                                                           user(std::move(userIn)),
                                                                                           func(std::move(funcIn)) {
    }

    KnativeNativeEndpoint::KnativeNativeEndpoint(
            std::string userIn,
            std::string funcIn,
            int port,
            int threadCount
    ) : Endpoint(port,
                 threadCount),
        user(std::move(userIn)),
        func(std::move(funcIn)) {
    }

    std::shared_ptr<Pistache::Http::Handler> KnativeNativeEndpoint::getHandler() {
        return Pistache::Http::make_handler<KnativeNativeHandler>(user, func);
    }
}
