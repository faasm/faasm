#pragma once

#include <endpoint/Endpoint.h>

namespace knative {
    class KnativeEndpoint : public endpoint::Endpoint {
    public:
        KnativeEndpoint();

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
