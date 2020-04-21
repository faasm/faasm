#pragma once

#include <endpoint/Endpoint.h>

namespace knative_native {
    class KnativeNativeEndpoint : public endpoint::Endpoint {
    public:
        KnativeNativeEndpoint(std::string userIn, std::string funcIn);

        KnativeNativeEndpoint(std::string userIn, std::string funcIn, int port, int threadCount);

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    private:
        const std::string user;
        const std::string func;
    };
}
