#include <codegen/MachineCodeGenerator.h>
#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/state/State.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <storage/FileLoader.h>
#include <upload/UploadEndpointHandler.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace upload {

using header = beast::http::field;
using BeastHttpRequest = faabric::util::BeastHttpRequest;
using BeastHttpResponse = faabric::util::BeastHttpResponse;

class PathParts
{
  public:
    PathParts(BeastHttpRequest requestIn)
      : request(requestIn)
    {
        auto uri = std::string(request.target());
        relativeUri = uri.substr(0, uri.find("?"));
        boost::trim_if(relativeUri, boost::is_any_of("/"));
        boost::split(pathParts,
                     relativeUri,
                     boost::is_any_of("/"),
                     boost::token_compress_on);
    }

    std::string relativeUri;

    std::string getPart(int idx)
    {
        if (pathParts.size() < idx + 1) {
            std::string errorMsg =
              fmt::format("Bad URL, expected single path part {}", relativeUri);
            throw InvalidPathException(errorMsg);
        }

        return pathParts[idx];
    }

  private:
    BeastHttpRequest request;
    std::vector<std::string> pathParts;
};

#define PATH_PART(varName, pathParts, idx)                                     \
    std::string varName;                                                       \
    {                                                                          \
        try {                                                                  \
            varName = pathParts.getPart(idx);                                  \
        } catch (InvalidPathException & e) {                                   \
            response.body() = e.what();                                        \
            response.result(beast::http::status::bad_request);                 \
            return;                                                            \
        }                                                                      \
    }

void UploadEndpointHandler::onRequest(
  faabric::endpoint::HttpRequestContext&& ctx,
  faabric::util::BeastHttpRequest&& request)
{
    SPDLOG_TRACE("Upload server received request");

    // Very permissive CORS
    faabric::util::BeastHttpResponse response;
    response.keep_alive(request.keep_alive());
    response.set(header::server, "Upload Server");
    response.set(header::access_control_allow_origin, "*");
    response.set(header::access_control_allow_methods, "GET,POST,PUT,OPTIONS");
    response.set(header::access_control_allow_headers,
                 "User-Agent,Content-Type");

    // Text response type
    response.set(header::content_type, "text/plain");

    switch (request.method()) {
        case boost::beast::http::verb::get: {
            handleGet(std::move(request), response);
            return ctx.sendFunction(std::move(response));
        }
        case boost::beast::http::verb::put: {
            handlePut(std::move(request), response);
            return ctx.sendFunction(std::move(response));
        }
        default: {
            throw std::runtime_error("Unrecognised request method");
        }
    };
}

void UploadEndpointHandler::handleGet(BeastHttpRequest&& request,
                                      BeastHttpResponse& response)
{
    std::string uri = std::string(request.target());
    std::string relativeUri = uri.substr(0, uri.find("?"));

    // Shortcut for ping
    if (relativeUri == "/ping") {
        SPDLOG_DEBUG("Responding to ping request");
        response.body() = std::string("PONG");
        response.result(beast::http::status::ok);
        return;
    }

    PathParts pathParts(request);

    auto& fileLoader = storage::getFileLoaderWithoutLocalCache();
    PATH_PART(pathType, pathParts, 0);
    std::vector<uint8_t> returnBytes;

    if (pathType == STATE_URL_PART) {
        SPDLOG_DEBUG("GET request for state at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(key, pathParts, 2);

        SPDLOG_INFO("Downloading state from ({}/{})", user, key);

        auto& state = faabric::state::getGlobalState();
        size_t stateSize = state.getStateSize(user, key);
        const auto& kvStore = state.getKV(user, key, stateSize);
        uint8_t* stateValue = kvStore->get();

        returnBytes = std::vector<uint8_t>(stateValue, stateValue + stateSize);

    } else if (pathType == SHARED_FILE_URL_PART) {
        SPDLOG_DEBUG("GET request for shared file at {}",
                     pathParts.relativeUri);

        auto itr = request.find(FILE_PATH_HEADER);
        if (itr != request.end()) {
            auto filePath = itr->value();
            returnBytes = fileLoader.loadSharedFile(filePath);
        } else {
            std::string errorMsg = fmt::format(
              "Bad request, expected file path header {}", FILE_PATH_HEADER);

            response.body() = errorMsg;
            response.result(beast::http::status::bad_request);
            return;
        }
    } else {
        std::string errorMsg =
          fmt::format("Unrecognised GET request to {}", pathParts.relativeUri);

        response.body() = errorMsg;
        response.result(beast::http::status::bad_request);
        return;
    }

    if (returnBytes.empty()) {
        response.result(beast::http::status::internal_server_error);
        response.body() = EMPTY_FILE_RESPONSE;
    } else {
        response.result(beast::http::status::ok);
        response.body() = std::string(returnBytes.begin(), returnBytes.end());
    }
}

void UploadEndpointHandler::handlePut(
  faabric::util::BeastHttpRequest&& request,
  faabric::util::BeastHttpResponse& response)
{
    PathParts pathParts(request);
    PATH_PART(pathType, pathParts, 0);

    if (pathType == STATE_URL_PART) {
        SPDLOG_DEBUG("PUT request for state at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(key, pathParts, 2);

        SPDLOG_INFO("Uploading state to ({}/{})", user, key);

        if (!request.body().empty()) {
            const std::vector<uint8_t> bytesData =
              faabric::util::stringToBytes(request.body());

            auto& state = faabric::state::getGlobalState();
            const auto& kvStore = state.getKV(user, key, bytesData.size());
            kvStore->set(bytesData.data());
            kvStore->pushFull();
        }

        response.result(beast::http::status::ok);
    } else if (pathType == PYTHON_URL_PART) {
        SPDLOG_DEBUG("PUT request for Python function at {}",
                     pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(function, pathParts, 2);

        faabric::Message msg;
        msg.set_ispython(true);
        msg.set_pythonuser(user);
        msg.set_pythonfunction(function);
        msg.set_inputdata(request.body());

        SPDLOG_INFO("Uploading Python function {}/{}",
                    msg.pythonuser(),
                    msg.pythonfunction());

        // Do the upload
        auto& fileLoader = storage::getFileLoaderWithoutLocalCache();
        fileLoader.uploadPythonFunction(msg);

        response.result(beast::http::status::ok);
    } else if (pathType == SHARED_FILE_URL_PART) {
        SPDLOG_DEBUG("PUT request for shared file at {}",
                     pathParts.relativeUri);

        auto itr = request.find(FILE_PATH_HEADER);
        if (itr != request.end()) {
            auto filePath = itr->value();

            if (!request.body().empty()) {
                auto data = faabric::util::stringToBytes(request.body());

                auto& fileLoader = storage::getFileLoaderWithoutLocalCache();
                fileLoader.uploadSharedFile(filePath, data);

                response.result(beast::http::status::ok);
            }
        } else {
            std::string errorMsg = fmt::format(
              "Bad request, expected file path header {}", FILE_PATH_HEADER);

            response.body() = errorMsg;
            response.result(beast::http::status::bad_request);
        }
    } else if (pathType == FUNCTION_URL_PART) {
        SPDLOG_DEBUG("PUT request for function at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(function, pathParts, 2);

        auto msg = faabric::util::messageFactory(user, function);
        SPDLOG_INFO("Uploading {}", faabric::util::funcToString(msg, false));
        msg.set_inputdata(request.body());

        auto& fileLoader = storage::getFileLoaderWithoutLocalCache();
        fileLoader.uploadFunction(msg);

        auto& gen = codegen::getMachineCodeGenerator(fileLoader);
        // When uploading a function, we always want to re-run the code
        // generation so we set the clean flag to true
        gen.codegenForFunction(msg, true);

        response.body() = std::string("Function upload complete\n");
        response.result(beast::http::status::ok);
    } else {
        std::string errorMsg =
          fmt::format("Unrecognised PUT request to {}", pathParts.relativeUri);
        SPDLOG_ERROR(errorMsg);

        response.body() = errorMsg;
        response.result(beast::http::status::bad_request);
    }
}
}
