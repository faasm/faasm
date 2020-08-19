#pragma once

#include <endpoint/Endpoint.h>
#include <util/config.h>

namespace faaslet {
    class FaasletEndpoint : public endpoint::Endpoint {
    public:
        FaasletEndpoint();

        FaasletEndpoint(int port, int threadCount);

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
