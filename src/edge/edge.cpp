#include "edge/edge.h"
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
            request.reply(status_codes::OK, "Invalid path. Must be /f|fa/<user>/<func>/ or /s/<user>/<key>/\n");
            throw InvalidPathException();
        }

        return pathParts;
    }

    void RestServer::listen(const std::string &port) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string addr = "http://0.0.0.0:" + port;
        http_listener listener(addr);

        listener.support(methods::GET, RestServer::handleGet);

        listener.support(methods::POST, RestServer::handlePost);

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
        if (pathParts[0] == "s") {
            const std::vector<uint8_t> stateBytes = getState(request);

            http_response response(status_codes::OK);
            response.set_body(stateBytes);
            request.reply(response);
        } else {
            request.reply(status_codes::OK, "Invalid path. Can only GET state at /s/<user>/<key/ \n");
            throw InvalidPathException();
        }
    }

    void RestServer::handlePost(const http_request &request) {
        const std::chrono::steady_clock::time_point &t = prof::startTimer();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::FunctionCall call = RestServer::buildCallFromRequest(request);

        infra::Redis *redis = infra::Redis::getThreadConnection();

        redis->callFunction(call);

        prof::logEndTimer("edge-submit", t);

        if (call.isasync()) {
            // Don't wait for result
            logger->info("Async request {}", infra::funcToString(call));
            request.reply(status_codes::Created, "Async request submitted\n");
        } else {
            logger->info("Sync request {}", infra::funcToString(call));
            message::FunctionCall result = redis->getFunctionResult(call);

            const std::chrono::steady_clock::time_point &t2 = prof::startTimer();

            request.reply(status_codes::OK, result.outputdata() + "\n");
            logger->info("Finished request {}", infra::funcToString(call));

            prof::logEndTimer("edge-reply", t2);
        }
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

        infra::Redis *redis = infra::Redis::getThreadConnection();
        const std::vector<uint8_t> value = redis->get(realKey);

        return value;
    }

    void RestServer::handleStateUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::vector<std::string> pathParts = RestServer::getPathParts(request);
        std::string user = pathParts[1];
        std::string key = pathParts[2];
        std::string realKey = user + "_" + key;

        logger->info("Upload state to ({}/{})", user, key);

        infra::Redis *redis = infra::Redis::getThreadConnection();

        // Read request body into KV store
        const concurrency::streams::istream bodyStream = request.body();
        concurrency::streams::stringstreambuf inputStream;
        bodyStream.read_to_end(inputStream).then([&inputStream, &redis, &realKey](size_t size) {
            if (size > 0) {
                std::string s = inputStream.collection();
                const std::vector<uint8_t> bytesData = util::stringToBytes(s);
                redis->set(realKey, bytesData);
            }

        }).wait();

        request.reply(status_codes::OK, "State upload complete\n");
    }

    void RestServer::handleFunctionUpload(const http_request &request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::FunctionCall call = RestServer::buildCallFromRequest(request);
        std::string outputFile = infra::getFunctionFile(call);

        logger->info("Uploading {}", infra::funcToString(call));

        // Here the call input data is actually the file
        std::ofstream out(outputFile);
        const std::string &fileBody = call.inputdata();
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        wasm::WasmModule::compileToObjectFile(call);

        request.reply(status_codes::OK, "Function upload complete\n");
    }

    message::FunctionCall RestServer::buildCallFromRequest(const http_request &request) {
        const std::vector<std::string> pathParts = RestServer::getPathParts(request);

        if (pathParts.size() != 3) {
            request.reply(status_codes::OK, "Invalid path (must be /f|fa/<user>/<func>/ \n");
            throw InvalidPathException();
        }

        // Check URI
        if (pathParts[0] == "f" || pathParts[0] == "fa") {
            message::FunctionCall call;
            call.set_user(pathParts[1]);
            call.set_function(pathParts[2]);
            call.set_isasync(pathParts[0] == "fa");

            // Read request into call input data
            const concurrency::streams::istream bodyStream = request.body();
            concurrency::streams::stringstreambuf inputStream;
            bodyStream.read_to_end(inputStream).then([&inputStream, &call](size_t size) {
                if (size > 0) {
                    std::string s = inputStream.collection();
                    call.set_inputdata(s);
                }

            }).wait();

            return call;
        }

        throw InvalidPathException();
    }
};

