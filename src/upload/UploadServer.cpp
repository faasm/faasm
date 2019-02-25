#include "UploadServer.h"

#include <util/logging.h>
#include <util/bytes.h>

#include <redis/Redis.h>
#include <aws/S3Wrapper.h>
#include <wasm/FunctionLoader.h>


namespace edge {
    UploadServer::UploadServer() = default;

    std::vector<std::string> UploadServer::getPathParts(const http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

        bool isValid = true;
        if (pathParts.size() != 3) {
            isValid = false;
        }

        if (isValid && !(pathParts[0] == "f" || pathParts[0] == "fa" || pathParts[0] == "s")) {
            isValid = false;
        }

        if (!isValid) {
            request.reply(status_codes::OK, "Invalid path. Must be /s/<user>/<key>/\n");
            throw InvalidPathException();
        }

        return pathParts;
    }

    void UploadServer::listen(const std::string &port) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string addr = "http://0.0.0.0:" + port;
        http_listener listener(addr);

        listener.support(methods::GET, UploadServer::handleGet);

        listener.support(methods::PUT, UploadServer::handlePut);

        listener.open().wait();

        // Continuous loop required to allow listening apparently
        logger->info("Listening for requests on localhost:{}", port);
        while (true) {
            usleep(60 * 1000 * 1000);
        }
    }

    void UploadServer::handleGet(const http_request &request) {
        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        const std::vector<uint8_t> stateBytes = getState(request);

        http_response response(status_codes::OK);
        response.set_body(stateBytes);
        request.reply(response);
    }

    void UploadServer::handlePut(const http_request &request) {
        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);
        if (pathParts[0] == "s") {
            handleStateUpload(request);
        } else {
            handleFunctionUpload(request);
        }
    }

    std::vector<uint8_t> UploadServer::getState(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = user + "_" + key;

        logger->info("Downloading state from ({}/{})", user, key);

        redis::Redis &redis = redis::Redis::getState();
        const std::vector<uint8_t> value = redis.get(realKey);

        return value;
    }

    void UploadServer::handleStateUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = user + "_" + key;

        logger->info("Upload state to ({}/{})", user, key);

        redis::Redis &redis = redis::Redis::getState();

        // Read request body into KV store
        const concurrency::streams::istream bodyStream = request.body();
        concurrency::streams::stringstreambuf inputStream;
        bodyStream.read_to_end(inputStream).then([&inputStream, &redis, &realKey](size_t size) {
            if (size > 0) {
                std::string s = inputStream.collection();
                const std::vector<uint8_t> bytesData = util::stringToBytes(s);
                redis.set(realKey, bytesData);
            }

        }).wait();

        request.reply(status_codes::OK, "State upload complete\n");
    }

    void UploadServer::handleFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message msg = UploadServer::buildMessageFromRequest(request);
        logger->info("Uploading {}", util::funcToString(msg));

        // Do the upload
        wasm::FunctionLoader &l = wasm::getFunctionLoader();
        l.uploadFunction(msg);

        request.reply(status_codes::OK, "Function upload complete\n");
    }

    message::Message UploadServer::buildMessageFromRequest(const http_request &request) {
        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        if (pathParts.size() != 3) {
            request.reply(status_codes::OK, "Invalid path (must be /f|fa/<user>/<func>/ \n");
            throw InvalidPathException();
        }

        // Check URI
        if (pathParts[0] == "f" || pathParts[0] == "fa") {
            message::Message msg;
            msg.set_user(pathParts[1]);
            msg.set_function(pathParts[2]);
            msg.set_isasync(pathParts[0] == "fa");

            // Read request into msg input data
            const concurrency::streams::istream bodyStream = request.body();
            concurrency::streams::stringstreambuf inputStream;
            bodyStream.read_to_end(inputStream).then([&inputStream, &msg](size_t size) {
                if (size > 0) {
                    std::string s = inputStream.collection();
                    msg.set_inputdata(s);
                }

            }).wait();

            return msg;
        }

        throw InvalidPathException();
    }
};

