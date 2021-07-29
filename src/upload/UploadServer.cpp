#include "UploadServer.h"

#include <faabric/state/State.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>

#define FUNCTION_UPLOAD_PART "f"
#define PYTHON_UPLOAD_PART "p"
#define STATE_UPLOAD_PART "s"
#define SHARED_FILE_UPLOAD_PART "file"

namespace edge {
void setPermissiveHeaders(http_response& response)
{
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"),
                           U("GET, POST, PUT, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"),
                           U("Accept,Content-Type"));
}

std::string getHeaderFromRequest(const http_request& request,
                                 const std::string& key)
{
    http_headers headers = request.headers();
    if (headers.has(key)) {
        return headers[key];
    }

    return "";
}

std::vector<std::string> UploadServer::getPathParts(const http_request& request)
{
    const uri uri = request.relative_uri();
    const std::vector<std::string> pathParts =
      uri::split_path(uri::decode(uri.path()));

    // Detect valid URLs
    if (pathParts.size() == 3) {
        // Check if one of the valid path types
        std::string pathType = pathParts[0];
        std::vector<std::string> validTypes = { FUNCTION_UPLOAD_PART,
                                                STATE_UPLOAD_PART,
                                                PYTHON_UPLOAD_PART };

        if (std::find(validTypes.begin(), validTypes.end(), pathType) !=
            validTypes.end()) {
            return pathParts;
        }

    } else if (!pathParts.empty() && pathParts[0] == SHARED_FILE_UPLOAD_PART) {
        return pathParts;
    }

    request.reply(status_codes::BadRequest, "Invalid path\n");
    throw InvalidPathException("Invalid request path");
}

void UploadServer::listen(const std::string& port)
{
    std::string addr = "http://0.0.0.0:" + port;
    http_listener listener(addr);

    listener.support(methods::GET, UploadServer::handleGet);

    listener.support(methods::OPTIONS, UploadServer::handleOptions);

    listener.support(methods::PUT, UploadServer::handlePut);

    listener.open().wait();

    // Continuous loop required to allow listening apparently
    SPDLOG_INFO("Listening for requests on localhost:{}", port);
    while (!stopped) {
        SLEEP_MS(2000);
    }
}

void UploadServer::stop()
{
    stopped = true;
}

void UploadServer::handleGet(const http_request& request)
{
    const std::vector<std::string> pathParts =
      UploadServer::getPathParts(request);

    std::string uri = request.absolute_uri().to_string();

    storage::FileLoader& l = storage::getFileLoader();
    std::string pathType = pathParts[0];
    std::vector<uint8_t> returnBytes;

    if (pathType == STATE_UPLOAD_PART) {
        SPDLOG_DEBUG("GET request for state at {}", uri);
        returnBytes = getState(request);

    } else if (pathType == SHARED_FILE_UPLOAD_PART) {
        std::string filePath = getHeaderFromRequest(request, FILE_PATH_HEADER);

        if(filePath.empty()) {
            SPDLOG_ERROR("Shared file path empty");
            request.reply(status_codes::BadRequest, "Shared file path empty");
            return;
        }

        SPDLOG_DEBUG("GET request for shared file {} at {}", filePath, uri);
        returnBytes = l.loadSharedFile(filePath);

    } else {
        std::string errMessage =
          fmt::format("Unrecognised GET request to {}", uri);
        request.reply(status_codes::BadRequest, errMessage);
        return;
    }

    http_response response;
    if (returnBytes.empty()) {
        response = http_response(status_codes::InternalError);
        response.set_body(EMPTY_FILE_RESPONSE);
    } else {
        response = http_response(status_codes::OK);
        response.set_body(returnBytes);
    }

    setPermissiveHeaders(response);

    request.reply(response);
}

void UploadServer::handlePut(const http_request& request)
{
    SPDLOG_DEBUG("PUT request to {}", request.absolute_uri().to_string());

    std::string uri = request.absolute_uri().to_string();

    const std::vector<std::string> pathParts =
      UploadServer::getPathParts(request);
    std::string pathType = pathParts[0];
    if (pathType == STATE_UPLOAD_PART) {
        handleStateUpload(request);
    } else if (pathType == PYTHON_UPLOAD_PART) {
        handlePythonFunctionUpload(request);
    } else if (pathType == SHARED_FILE_UPLOAD_PART) {
        handleSharedFileUpload(request);
    } else if (pathType == FUNCTION_UPLOAD_PART) {
        handleFunctionUpload(request);
    } else {
        std::string errMessage =
          fmt::format("Unrecognised PUT request to {}", uri);
        request.reply(status_codes::BadRequest, errMessage);
    }
}

void UploadServer::handleOptions(const http_request& request)
{
    SPDLOG_DEBUG("OPTIONS request to {}", request.absolute_uri().to_string());

    http_response response(status_codes::OK);
    setPermissiveHeaders(response);
    request.reply(response);
}

std::vector<uint8_t> UploadServer::getState(const http_request& request)
{
    const std::vector<std::string> pathParts =
      UploadServer::getPathParts(request);
    std::string user = pathParts[1];
    std::string key = pathParts[2];
    SPDLOG_INFO("Downloading state from ({}/{})", user, key);

    faabric::state::State& state = faabric::state::getGlobalState();
    size_t stateSize = state.getStateSize(user, key);
    const std::shared_ptr<faabric::state::StateKeyValue>& kv =
      state.getKV(user, key, stateSize);
    uint8_t* stateValue = kv->get();

    const std::vector<uint8_t> value(stateValue, stateValue + stateSize);
    return value;
}

void UploadServer::handleStateUpload(const http_request& request)
{
    const std::vector<std::string> pathParts =
      UploadServer::getPathParts(request);
    std::string user = pathParts[1];
    std::string key = pathParts[2];

    SPDLOG_INFO("Upload state to ({}/{})", user, key);

    // Read request body into KV store
    const concurrency::streams::istream bodyStream = request.body();
    concurrency::streams::stringstreambuf inputStream;
    bodyStream.read_to_end(inputStream)
      .then([&inputStream, &user, &key](size_t size) {
          if (size > 0) {
              std::string s = inputStream.collection();
              const std::vector<uint8_t> bytesData =
                faabric::util::stringToBytes(s);

              faabric::state::State& state = faabric::state::getGlobalState();
              const std::shared_ptr<faabric::state::StateKeyValue>& kv =
                state.getKV(user, key, bytesData.size());
              kv->set(bytesData.data());
              kv->pushFull();
          }
      })
      .wait();

    http_response response(status_codes::OK);
    setPermissiveHeaders(response);
    response.set_body("State upload complete\n");
    request.reply(response);
}

void UploadServer::handlePythonFunctionUpload(const http_request& request)
{
    faabric::Message msg = UploadServer::buildMessageFromRequest(request);
    SPDLOG_INFO("Uploading Python function {}",
                faabric::util::funcToString(msg, false));

    // Do the upload
    storage::FileLoader& l = storage::getFileLoader();
    l.uploadPythonFunction(msg);

    request.reply(status_codes::OK, "Python function upload complete\n");
}

void UploadServer::handleSharedFileUpload(const http_request& request)
{
    std::string filePath = getHeaderFromRequest(request, FILE_PATH_HEADER);
    SPDLOG_INFO("Uploading shared file {}", filePath);

    const concurrency::streams::istream bodyStream = request.body();
    concurrency::streams::stringstreambuf inputStream;
    bodyStream.read_to_end(inputStream)
      .then([&inputStream, &filePath](size_t size) {
          if (size > 0) {
              std::string s = inputStream.collection();
              const std::vector<uint8_t> bytesData =
                faabric::util::stringToBytes(s);
              storage::FileLoader& l = storage::getFileLoader();
              l.uploadSharedFile(filePath, bytesData);
          }
      })
      .wait();

    request.reply(status_codes::OK, "Shared file uploaded\n");
}

void UploadServer::handleFunctionUpload(const http_request& request)
{
    faabric::Message msg = UploadServer::buildMessageFromRequest(request);
    SPDLOG_INFO("Uploading {}", faabric::util::funcToString(msg, false));

    // Do the upload
    storage::FileLoader& l = storage::getFileLoader();
    l.uploadFunction(msg);

    codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();
    gen.codegenForFunction(msg);

    request.reply(status_codes::OK, "Function upload complete\n");
}

faabric::Message UploadServer::buildMessageFromRequest(
  const http_request& request)
{
    const std::vector<std::string> pathParts =
      UploadServer::getPathParts(request);

    if (pathParts.size() != 3) {
        std::string uri = request.absolute_uri().to_string();
        std::string errMessage =
          fmt::format("Invalid path {}, must be /[fp]/<user>/<func>/", uri);

        SPDLOG_ERROR(errMessage);

        request.reply(status_codes::BadRequest, errMessage);
        throw InvalidPathException(errMessage);
    }

    // Check URI
    faabric::Message msg =
      faabric::util::messageFactory(pathParts[1], pathParts[2]);

    // Read request into msg input data
    const concurrency::streams::istream bodyStream = request.body();
    concurrency::streams::stringstreambuf inputStream;
    bodyStream.read_to_end(inputStream)
      .then([&inputStream, &msg](size_t size) {
          if (size > 0) {
              std::string s = inputStream.collection();
              msg.set_inputdata(s);
          }
      })
      .wait();

    return msg;
}
}
