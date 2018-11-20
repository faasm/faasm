#pragma once

#include <wasm/wasm.h>
#include <proto/faasm.pb.h>
#include <infra/infra.h>
#include <util/util.h>

#include <cpprest/http_listener.h>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

using namespace web::http::experimental::listener;
using namespace web::http;

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
        Rest::Router router;

        void setupRoutes();

        void handleFunction(const Rest::Request &request, Http::ResponseWriter response);

        void handleAsyncFunction(const Rest::Request &request, Http::ResponseWriter response);

        message::FunctionCall buildCallFromRequest(const Rest::Request &request);
    };

    class RestServer {
    public:
        RestServer();

        void listen(const std::string &port);

        static void handleGet(const http_request &request);

        static void handlePut(const http_request &request);

        static message::FunctionCall buildCallFromRequest(const http_request &request);

        static std::vector<std::string> getPathParts(const http_request &request);

        static std::vector<uint8_t> getState(const http_request &request);

    private:
        static void handleFunctionUpload(const http_request &request);

        static void handleStateUpload(const http_request &request);
    };

    class InvalidPathException : public std::exception {
    };
}