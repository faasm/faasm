#pragma once
#include <pistache/http.h>
#include <pistache/endpoint.h>

namespace http {
    class HttpEndpoint {
    public:
        HttpEndpoint(int port, int threadCount);
        void start();

        virtual void setHandler() = 0;
    protected:
        std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    };
}
