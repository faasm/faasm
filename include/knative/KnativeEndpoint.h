#pragma once

#include <endpoint/Endpoint.h>
#include <util/config.h>

namespace knative {
    class KnativeEndpoint : public endpoint::Endpoint {
    public:
        KnativeEndpoint();

        KnativeEndpoint(int port, int threadCount);

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
