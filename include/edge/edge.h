#pragma once

#include <wasm/wasm.h>
#include <proto/faasm.pb.h>
#include <infra/infra.h>
#include <util/util.h>

#include <cpprest/http_listener.h>

using namespace web::http::experimental::listener;
using namespace web::http;

namespace edge {
    class RestServer {
    public:
        RestServer();

        void listen(const std::string &port);

        static void handleGet(http_request request);
        static void handlePost(http_request request);
        static void handlePut(http_request request);

        static message::FunctionCall buildCallFromRequest(http_request &request);
    };


    class InvalidPathException : public std::exception {
    };

}