#include "UploadServer.h"

#include <util/logging.h>
#include <util/bytes.h>

#include <redis/Redis.h>
#include <storage/FunctionLoader.h>
#include <util/config.h>
#include <util/state.h>


namespace edge {
    UploadServer::UploadServer() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();
        if(conf.functionStorage == "fileserver") {
            logger->info("Overriding fileserver storage on upload server (as this is the fileserver)");
            conf.functionStorage = "local";
        }
    }

    std::vector<std::string> UploadServer::getPathParts(const http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

        bool isValid = false;
        if (pathParts.size() == 3) {
            isValid = true;
        }

        if (isValid) {
            // Check if one of the valid path types
            std::string pathType = pathParts[0];
            std::vector<std::string> validTypes = {"f", "fa", "s", "p", "pa"};

            if (std::find(validTypes.begin(), validTypes.end(), pathType) != validTypes.end()) {
                isValid = true;
            }
        }

        if (!isValid) {
            request.reply(status_codes::OK, "Invalid path\n");
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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("GET request to {}", request.absolute_uri().to_string());

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        storage::FunctionLoader &l = storage::getFunctionLoader();
        std::string pathType = pathParts[0];
        std::vector<uint8_t> returnBytes;
        message::Message msg = UploadServer::buildMessageFromRequest(request);

        if (pathType == "s") {
            returnBytes = getState(request);
        } else if (pathType == "p" || pathType == "pa") {
            returnBytes = l.loadPythonFunctionFile(msg);
        } else if (pathType == "fo"){
            returnBytes = l.loadFunctionObjectFile(msg);
        } else {
            returnBytes = l.loadFunctionWasm(msg);
        }

        if(returnBytes.empty()) {
            http_response response(status_codes::InternalError);
            response.set_body("Empty response");
            request.reply(response);
        } else {
            http_response response(status_codes::OK);
            response.set_body(returnBytes);
            request.reply(response);
        }
    }

    void UploadServer::handlePut(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("PUT request to {}", request.absolute_uri().to_string());

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);
        std::string pathType = pathParts[0];
        if (pathType == "s") {
            handleStateUpload(request);
        } else if (pathType == "p" || pathType == "pa") {
            handlePythonFunctionUpload(request);
        } else {
            handleFunctionUpload(request);
        }
    }

    std::vector<uint8_t> UploadServer::getState(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = util::keyForUser(user, key);

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
        std::string realKey = util::keyForUser(user, key);

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

    void UploadServer::handlePythonFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message msg = UploadServer::buildMessageFromRequest(request);
        logger->info("Uploading Python function {}", util::funcToString(msg, false));

        // Do the upload
        storage::FunctionLoader &l = storage::getFunctionLoader();
        l.uploadPythonFunction(msg);

        request.reply(status_codes::OK, "Python function upload complete\n");
    }

    void UploadServer::handleFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message msg = UploadServer::buildMessageFromRequest(request);
        logger->info("Uploading {}", util::funcToString(msg, false));

        // Do the upload
        storage::FunctionLoader &l = storage::getFunctionLoader();
        l.uploadFunction(msg);

        request.reply(status_codes::OK, "Function upload complete\n");
    }

    message::Message UploadServer::buildMessageFromRequest(const http_request &request) {
        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        if (pathParts.size() != 3) {
            request.reply(status_codes::OK, "Invalid path (must be /f|fa|p|pa/<user>/<func>/ \n");
            throw InvalidPathException();
        }

        // Check URI
        message::Message msg;
        msg.set_user(pathParts[1]);
        msg.set_function(pathParts[2]);
        msg.set_isasync(pathParts[0] == "fa" || pathParts[0] == "pa");

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
};

