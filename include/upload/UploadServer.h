#pragma once

#include <faabric/util/func.h>

#include <cpprest/http_listener.h>

using namespace web::http::experimental::listener;
using namespace web::http;

#define UPLOAD_PORT "8002"

namespace edge {
class UploadServer
{
  public:
    UploadServer();

    void listen(const std::string& port);

    void stop();

    static void handleGet(const http_request& request);

    static void handlePut(const http_request& request);

    static void handleOptions(const http_request& request);

    static faabric::Message buildMessageFromRequest(
      const http_request& request);

    static std::vector<std::string> getPathParts(const http_request& request);

    static std::vector<uint8_t> getState(const http_request& request);

  private:
    bool stopped = false;

    static void handlePythonFunctionUpload(const http_request& request);

    static void handleFunctionUpload(const http_request& request);

    static void handleStateUpload(const http_request& request);

    static void handleSharedFileUpload(const http_request& request);
};

class InvalidPathException : public faabric::util::FaabricException
{
  public:
    explicit InvalidPathException(std::string message)
      : faabric::util::FaabricException(std::move(message))
    {}
};
}
