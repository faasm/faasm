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

namespace edge {

// --------------------------------------
// REQUEST UTILS
// --------------------------------------

class PathParts
{
  public:
    PathParts(http_request requestIn)
      : request(requestIn)
    {
        relativeUri = uri::decode(request.relative_uri().path());
        pathParts = uri::split_path(relativeUri);
    }

    std::string relativeUri;

    std::string getPart(int idx)
    {
        if (pathParts.size() < idx + 1) {
            std::string errorMsg =
              fmt::format("Bad URL, expected single path part {}", relativeUri);
            request.reply(status_codes::BadRequest, errorMsg);
            throw InvalidPathException(errorMsg);
        }

        return pathParts[idx];
    }

  private:
    http_request request;
    std::vector<std::string> pathParts;
};

#define PATH_PART(varName, pathParts, idx)                                     \
    std::string varName;                                                       \
    {                                                                          \
        try {                                                                  \
            varName = pathParts.getPart(idx);                                  \
        } catch (InvalidPathException & e) {                                   \
            return;                                                            \
        }                                                                      \
    }

#define PATH_HEADER(varName, request)                                          \
    std::string varName;                                                       \
    {                                                                          \
        http_headers headers = request.headers();                              \
        if (headers.has(FILE_PATH_HEADER)) {                                   \
            varName = headers[FILE_PATH_HEADER];                               \
        } else {                                                               \
            std::string errorMsg = fmt::format(                                \
              "Bad request, expected file path header {}", FILE_PATH_HEADER);  \
            request.reply(status_codes::BadRequest, errorMsg);                 \
            return;                                                            \
        }                                                                      \
    }

void setPermissiveHeaders(http_response& response)
{
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"),
                           U("GET, POST, PUT, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"),
                           U("Accept,Content-Type"));
}

// --------------------------------------
// SERVER LIFECYCLE
// --------------------------------------

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

// --------------------------------------
// GET REQUESTS
// --------------------------------------

void UploadServer::handleGet(const http_request& request)
{
    // Shortcut for ping
    std::string relativeUri = uri::decode(request.relative_uri().path());
    if (relativeUri == "/ping") {
        SPDLOG_DEBUG("Responding to ping request");
        http_response response(status_codes::OK);
        response.set_body("PONG");
        setPermissiveHeaders(response);
        request.reply(response);
        return;
    }

    PathParts pathParts(request);

    storage::FileLoader& l = storage::getFileLoaderWithoutLocalCache();
    PATH_PART(pathType, pathParts, 0);
    std::vector<uint8_t> returnBytes;

    if (pathType == STATE_URL_PART) {
        SPDLOG_DEBUG("GET request for state at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(key, pathParts, 2);
        returnBytes = getState(user, key);

    } else if (pathType == SHARED_FILE_URL_PART) {
        SPDLOG_DEBUG("GET request for shared file at {}",
                     pathParts.relativeUri);

        PATH_HEADER(filePath, request);
        returnBytes = l.loadSharedFile(filePath);

    } else {
        std::string errMessage =
          fmt::format("Unrecognised GET request to {}", pathParts.relativeUri);
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

std::vector<uint8_t> UploadServer::getState(const std::string& user,
                                            const std::string& key)
{
    SPDLOG_INFO("Downloading state from ({}/{})", user, key);

    faabric::state::State& state = faabric::state::getGlobalState();
    size_t stateSize = state.getStateSize(user, key);
    const std::shared_ptr<faabric::state::StateKeyValue>& kv =
      state.getKV(user, key, stateSize);
    uint8_t* stateValue = kv->get();

    const std::vector<uint8_t> value(stateValue, stateValue + stateSize);
    return value;
}

// --------------------------------------
// OPTIONS REQUESTS
// --------------------------------------

void UploadServer::handleOptions(const http_request& request)
{
    SPDLOG_DEBUG("OPTIONS request to {}", request.absolute_uri().to_string());

    http_response response(status_codes::OK);
    setPermissiveHeaders(response);
    request.reply(response);
}

// --------------------------------------
// PUT REQUESTS
// --------------------------------------

void UploadServer::handlePut(const http_request& request)
{
    PathParts pathParts(request);
    PATH_PART(pathType, pathParts, 0);

    if (pathType == STATE_URL_PART) {
        SPDLOG_DEBUG("PUT request for state at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(key, pathParts, 2);
        handleStateUpload(request, user, key);

    } else if (pathType == PYTHON_URL_PART) {
        SPDLOG_DEBUG("PUT request for Python function at {}",
                     pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(function, pathParts, 2);
        handlePythonFunctionUpload(request, user, function);

    } else if (pathType == SHARED_FILE_URL_PART) {
        SPDLOG_DEBUG("PUT request for shared file at {}",
                     pathParts.relativeUri);

        PATH_HEADER(filePath, request);
        handleSharedFileUpload(request, filePath);

    } else if (pathType == FUNCTION_URL_PART) {
        SPDLOG_DEBUG("PUT request for function at {}", pathParts.relativeUri);

        PATH_PART(user, pathParts, 1);
        PATH_PART(function, pathParts, 2);
        handleFunctionUpload(request, user, function);

    } else {
        std::string errMessage =
          fmt::format("Unrecognised PUT request to {}", pathParts.relativeUri);
        request.reply(status_codes::BadRequest, errMessage);
    }
}

void UploadServer::handleStateUpload(const http_request& request,
                                     const std::string& user,
                                     const std::string& key)
{
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

void UploadServer::handlePythonFunctionUpload(const http_request& request,
                                              const std::string& user,
                                              const std::string& function)
{
    faabric::Message msg;
    msg.set_ispython(true);
    msg.set_pythonuser(user);
    msg.set_pythonfunction(function);
    UploadServer::extractRequestBody(request, msg);

    SPDLOG_INFO("Uploading Python function {}/{}",
                msg.pythonuser(),
                msg.pythonfunction());

    // Do the upload
    storage::FileLoader& l = storage::getFileLoaderWithoutLocalCache();
    l.uploadPythonFunction(msg);

    request.reply(status_codes::OK, "Python function upload complete\n");
}

void UploadServer::handleSharedFileUpload(const http_request& request,
                                          const std::string& path)
{
    SPDLOG_INFO("Uploading shared file {}", path);

    const concurrency::streams::istream bodyStream = request.body();
    concurrency::streams::stringstreambuf inputStream;
    bodyStream.read_to_end(inputStream)
      .then([&inputStream, &path](size_t size) {
          if (size > 0) {
              std::string s = inputStream.collection();
              const std::vector<uint8_t> bytesData =
                faabric::util::stringToBytes(s);
              storage::FileLoader& l =
                storage::getFileLoaderWithoutLocalCache();
              l.uploadSharedFile(path, bytesData);
          }
      })
      .wait();

    request.reply(status_codes::OK, "Shared file uploaded\n");
}

void UploadServer::handleFunctionUpload(const http_request& request,
                                        const std::string& user,
                                        const std::string& function)
{
    faabric::Message msg = faabric::util::messageFactory(user, function);
    UploadServer::extractRequestBody(request, msg);

    SPDLOG_INFO("Uploading {}", faabric::util::funcToString(msg, false));

    // Upload the WASM bytes using a file loader without cache to make sure we
    // always use the latest-uploaded WASM. We also want to make sure we use
    // the same file loader to generate the machine code
    storage::FileLoader& l = storage::getFileLoaderWithoutLocalCache();
    l.uploadFunction(msg);

    codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator(l);
    // When uploading a function, we always want to re-run the code generation
    // so we set the clean flag to true
    gen.codegenForFunction(msg, true);

    request.reply(status_codes::OK, "Function upload complete\n");
}

void UploadServer::extractRequestBody(const http_request& req,
                                      faabric::Message& msg)
{
    // Read request into msg input data
    const concurrency::streams::istream bodyStream = req.body();
    concurrency::streams::stringstreambuf inputStream;
    bodyStream.read_to_end(inputStream)
      .then([&inputStream, &msg](size_t size) {
          if (size > 0) {
              std::string s = inputStream.collection();
              msg.set_inputdata(s);
          }
      })
      .wait();
}
}
