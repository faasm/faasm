#pragma once
#include <pistache/http.h>
#include <pistache/endpoint.h>

using namespace Pistache;

namespace http {
    class HttpEndpoint {
    public:
        HttpEndpoint(int port, int threadCount);
        void start();

        virtual void setHandler() = 0;
    protected:
        std::shared_ptr<Http::Endpoint> httpEndpoint;
    };
}
