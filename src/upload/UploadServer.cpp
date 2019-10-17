#include "UploadServer.h"

#include <util/logging.h>
#include <util/bytes.h>

#include <redis/Redis.h>
#include <storage/FileLoader.h>
#include <util/config.h>
#include <util/state.h>
#include <util/files.h>
#include <state/State.h>


namespace edge {
    std::string getHeaderFromRequest(const http_request &request, const std::string &key) {
        http_headers headers = request.headers();
        if (headers.has(key)) {
            return headers[key];
        } else {
            return "";
        }
    }

    UploadServer::UploadServer() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.functionStorage == "fileserver") {
            logger->info("Overriding fileserver storage on upload server (as this is the fileserver)");
            conf.functionStorage = "local";
        }
    }

    std::vector<std::string> UploadServer::getPathParts(const http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

        // Detect valid URLs
        if (pathParts.size() == 3) {
            // Check if one of the valid path types
            std::string pathType = pathParts[0];
            std::vector<std::string> validTypes = {"f", "fo", "fa", "s", "p", "pa"};

            if (std::find(validTypes.begin(), validTypes.end(), pathType) != validTypes.end()) {
                return pathParts;
            }
        } else if (pathParts.size() == 1) {
            if (pathParts[0] == "sobjwasm" || pathParts[0] == "sobjobj" || pathParts[0] == "file") {
                return pathParts;
            }
        } else if(pathParts.size() > 0 && pathParts[0] == "file") {
            return pathParts;
        }

        request.reply(status_codes::OK, "Invalid path\n");
        throw InvalidPathException("Invalid request path");
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

        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        storage::FileLoader &l = storage::getFileLoader();
        std::string pathType = pathParts[0];
        std::vector<uint8_t> returnBytes;

        const utility::string_t &uri = request.absolute_uri().to_string();

        if (pathType == "sobjwasm" || pathType == "sobjobj" || pathType == "file") {
            std::string filePath = getHeaderFromRequest(request, FILE_PATH_HEADER);

            logger->debug("GET request to {} ({})", uri, filePath);
            if (pathType == "sobjwasm") {
                returnBytes = l.loadSharedObjectWasm(filePath);
            } else if (pathType == "sobjobj") {
                returnBytes = l.loadSharedObjectObjectFile(filePath);
            } else {
                returnBytes = l.loadSharedFile(filePath);
            }
        } else {
            logger->debug("GET request to {}", uri);

            message::Message msg = UploadServer::buildMessageFromRequest(request);

            if (pathType == "s") {
                returnBytes = getState(request);
            } else if (pathType == "p" || pathType == "pa") {
                returnBytes = l.loadPythonFunctionFile(msg);
            } else if (pathType == "fo") {
                returnBytes = l.loadFunctionObjectFile(msg);
            } else {
                returnBytes = l.loadFunctionWasm(msg);
            }
        }

        if (returnBytes.empty()) {
            http_response response(status_codes::InternalError);
            response.set_body(EMPTY_FILE_RESPONSE);
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
        } else if (pathType == "file") {
            handleSharedFileUpload(request);
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
        storage::FileLoader &l = storage::getFileLoader();
        l.uploadPythonFunction(msg);

        request.reply(status_codes::OK, "Python function upload complete\n");
    }

    void UploadServer::handleSharedFileUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::string filePath = getHeaderFromRequest(request, FILE_PATH_HEADER);
        logger->info("Uploading shared file {}", filePath);

        const concurrency::streams::istream bodyStream = request.body();
        concurrency::streams::stringstreambuf inputStream;
        bodyStream.read_to_end(inputStream).then([&inputStream, &filePath](size_t size) {
            if (size > 0) {
                std::string s = inputStream.collection();
                const std::vector<uint8_t> bytesData = util::stringToBytes(s);
                storage::FileLoader &l = storage::getFileLoader();
                l.uploadSharedFile(filePath, bytesData);
            }

        }).wait();

        request.reply(status_codes::OK, "Shared file uploaded\n");
    }

    void UploadServer::handleFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message msg = UploadServer::buildMessageFromRequest(request);
        logger->info("Uploading {}", util::funcToString(msg, false));

        // Do the upload
        storage::FileLoader &l = storage::getFileLoader();
        l.uploadFunction(msg);

        request.reply(status_codes::OK, "Function upload complete\n");
    }

    message::Message UploadServer::buildMessageFromRequest(const http_request &request) {
        const std::vector<std::string> pathParts = UploadServer::getPathParts(request);

        if (pathParts.size() != 3) {
            const char *msg = "Invalid path (must be /f|fa|p|pa/<user>/<func>/ \n";
            request.reply(status_codes::OK, msg);
            throw InvalidPathException(msg);
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
}

