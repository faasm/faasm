#pragma once

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <proto/faasm.pb.h>
#include <infra/infra.h>

#include <cpprest/http_listener.h>

using namespace Pistache;
using namespace web::http::experimental::listener;
using namespace web::http;

namespace edge {

    class FunctionEndpoint {
    public:
        explicit FunctionEndpoint(Address addr);

        void init(int threadCount);

        void start();

        void shutdown();

    private:
        std::shared_ptr<Http::Endpoint> httpEndpoint;
        std::shared_ptr<infra::RedisClient> redis;
        Rest::Router router;

        void setupRoutes();

        void status(const Rest::Request &request, Http::ResponseWriter response);

        void handleFunction(const Rest::Request &request, Http::ResponseWriter response);

        void handleAsyncFunction(const Rest::Request &request, Http::ResponseWriter response);

        message::FunctionCall buildCallFromRequest(const Rest::Request &request);
    };

    class RestServer {
    public:
        RestServer();

        void listen(const std::string &port);

        void handleGet(http_request request);

    private:
        std::string port;
        http_listener *listener;
    };

}