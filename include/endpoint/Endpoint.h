#pragma once
#include <pistache/http.h>
#include <pistache/endpoint.h>

namespace endpoint {
    class Endpoint {
    public:
        Endpoint(int port, int threadCount);
        void start();

        virtual std::shared_ptr<Pistache::Http::Handler> getHandler() = 0;
    private:
        int port;
        int threadCount;
    };
}
