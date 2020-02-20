#pragma once

#include <util/func.h>

#include <cpprest/http_listener.h>

using namespace web::http::experimental::listener;
using namespace web::http;

namespace edge {
    class UploadServer {
    public:
        UploadServer();

        void listen(const std::string &port);

        static void handleGet(const http_request &request);

        static void handlePut(const http_request &request);

        static void handleOptions(const http_request &request);

        static message::Message buildMessageFromRequest(const http_request &request);

        static std::vector<std::string> getPathParts(const http_request &request);

        static std::vector<uint8_t> getState(const http_request &request);

    private:
        static void handlePythonFunctionUpload(const http_request &request);

        static void handleFunctionUpload(const http_request &request);

        static void handleStateUpload(const http_request &request);

        static void handleSharedFileUpload(const http_request &request);
    };

    class InvalidPathException : public util::FaasmException {
    public:
        explicit InvalidPathException(std::string message): FaasmException(std::move(message)) {

        }
    };
}