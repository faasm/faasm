#pragma once

#include <faabric/util/func.h>

#include <cpprest/http_listener.h>

using namespace web::http::experimental::listener;
using namespace web::http;

#define UPLOAD_PORT "8002"

#define FUNCTION_URL_PART "f"
#define PYTHON_URL_PART "p"
#define STATE_URL_PART "s"
#define SHARED_FILE_URL_PART "file"

namespace edge {
class UploadServer
{
  public:
    void listen(const std::string& port);

    void stop();

    static void handleGet(const http_request& request);

    static void handlePut(const http_request& request);

    static void handleOptions(const http_request& request);

  private:
    bool stopped = false;

    static std::vector<uint8_t> getState(const std::string& user,
                                         const std::string& key);

    static void handlePythonFunctionUpload(const http_request& request,
                                           const std::string& user,
                                           const std::string& function);

    static void handleFunctionUpload(const http_request& request,
                                     const std::string& user,
                                     const std::string& function);

    static void handleStateUpload(const http_request& request,
                                  const std::string& user,
                                  const std::string& key);

    static void handleSharedFileUpload(const http_request& request,
                                       const std::string& path);

    static void extractRequestBody(const http_request& req,
                                   faabric::Message& msg);
};

class InvalidPathException : public faabric::util::FaabricException
{
  public:
    explicit InvalidPathException(std::string message)
      : faabric::util::FaabricException(std::move(message))
    {}
};
}
