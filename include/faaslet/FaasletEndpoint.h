#pragma once

#include <faabric/endpoint/Endpoint.h>
#include <faabric/util/config.h>

using namespace faabric::endpoint;

namespace faaslet {
    class FaasletEndpoint : public Endpoint {
    public:
        FaasletEndpoint();

        FaasletEndpoint(int port, int threadCount);

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
