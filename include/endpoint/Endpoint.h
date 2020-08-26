#pragma once
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <faabric/util/config.h>

namespace endpoint {
    class Endpoint {
    public:
        Endpoint() = default;

        Endpoint(int port, int threadCount);
        void start();

        virtual std::shared_ptr<Pistache::Http::Handler> getHandler() = 0;
    private:
        int port = faabric::util::getSystemConfig().endpointPort;
        int threadCount = faabric::util::getSystemConfig().endpointNumThreads;
    };
}
