#include "upload/upload.h"
#include "prof/prof.h"


namespace edge {
    RestServer::RestServer() = default;

    std::vector<std::string> RestServer::getPathParts(const http_request &request) {
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

    void RestServer::listen(const std::string &port) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string addr = "http://0.0.0.0:" + port;
        http_listener listener(addr);

        listener.support(methods::GET, RestServer::handleGet);

        listener.support(methods::PUT, RestServer::handlePut);

        listener.open().wait();

        // Continuous loop required to allow listening apparently
        logger->info("Listening for requests on localhost:{}", port);
        while (true) {
            usleep(60 * 1000 * 1000);
        }
    }

    void RestServer::handleGet(const http_request &request) {
        const std::vector<std::string> pathParts = RestServer::getPathParts(request);

        const std::vector<uint8_t> stateBytes = getState(request);

        http_response response(status_codes::OK);
        response.set_body(stateBytes);
        request.reply(response);
    }

    void RestServer::handlePut(const http_request &request) {
        const std::vector<std::string> pathParts = RestServer::getPathParts(request);
        if (pathParts[0] == "s") {
            handleStateUpload(request);
        } else {
            handleFunctionUpload(request);
        }
    }

    std::vector<uint8_t> RestServer::getState(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = RestServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = user + "_" + key;

        logger->info("Downloading state from ({}/{})", user, key);

        infra::Redis &redis = infra::Redis::getThreadState();
        const std::vector<uint8_t> value = redis.get(realKey);

        return value;
    }

    void RestServer::handleStateUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = RestServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = user + "_" + key;

        logger->info("Upload state to ({}/{})", user, key);

        infra::Redis &redis = infra::Redis::getThreadState();

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

    void RestServer::handleFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message msg = RestServer::buildMessageFromRequest(request);
        std::string outputFile = infra::getFunctionFile(msg);

        logger->info("Uploading {}", infra::funcToString(msg));

        // Here the msg input data is actually the file
        std::ofstream out(outputFile);
        const std::string &fileBody = msg.inputdata();
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        wasm::WasmModule::compileToObjectFile(msg);

        request.reply(status_codes::OK, "Function upload complete\n");
    }

    message::Message RestServer::buildMessageFromRequest(const http_request &request) {
        const std::vector<std::string> pathParts = RestServer::getPathParts(request);

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

