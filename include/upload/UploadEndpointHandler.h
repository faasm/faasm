#pragma once

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/util/exception.h>

// TODO: consider moving elsewhere
#define UPLOAD_PORT "8002"

#define FUNCTION_URL_PART "f"
#define PYTHON_URL_PART "p"
#define STATE_URL_PART "s"
#define SHARED_FILE_URL_PART "file"

namespace upload {
class UploadEndpointHandler final
  : public faabric::endpoint::HttpRequestHandler
  , public std::enable_shared_from_this<UploadEndpointHandler>
{
  public:
    void onRequest(faabric::endpoint::HttpRequestContext&& ctx,
                   faabric::util::BeastHttpRequest&& request) override;

  private:
    static void handleGet(faabric::util::BeastHttpRequest&& request,
                   faabric::util::BeastHttpResponse& response);

    static void handlePut(faabric::util::BeastHttpRequest&& request,
                   faabric::util::BeastHttpResponse& response);
};

class InvalidPathException : public faabric::util::FaabricException
{
  public:
    explicit InvalidPathException(std::string message)
      : faabric::util::FaabricException(std::move(message))
    {}
};
}
