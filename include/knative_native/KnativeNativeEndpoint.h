#pragma once

#include <endpoint/Endpoint.h>

namespace knative_native {
    class KnativeNativeEndpoint : public endpoint::Endpoint {
    public:
        KnativeNativeEndpoint();

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
