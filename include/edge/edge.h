#pragma once

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <redis/redis.h>

using namespace Pistache;

namespace edge {

    class FunctionEndpoint {
    public:
        explicit FunctionEndpoint(Address addr);

        void init(int threadCount);

        void start();

        void shutdown();
    private:
        std::shared_ptr<Http::Endpoint> httpEndpoint;
        std::shared_ptr<redis::RedisClient> redis;
        Rest::Router router;

        void setupRoutes();

        void status(const Rest::Request &request, Http::ResponseWriter response);

        void handleFunction(const Rest::Request &request, Http::ResponseWriter response);
    };
}