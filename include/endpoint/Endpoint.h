#pragma once
#include <pistache/http.h>
#include <pistache/endpoint.h>
#include <util/config.h>

namespace endpoint {
    class Endpoint {
    public:
        Endpoint() = default;

        Endpoint(int port, int threadCount);
        void start();

        virtual std::shared_ptr<Pistache::Http::Handler> getHandler() = 0;
    private:
        int port = util::getSystemConfig().endpointPort;
        int threadCount = util::getSystemConfig().endpointNumThreads;
    };
}
