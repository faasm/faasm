#include "edge/edge.h"


namespace edge {
    RestServer::RestServer() = default;

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

    void RestServer::handleGet(http_request request) {
        request.reply(status_codes::OK, "Get OK\n");
    }

    void RestServer::handlePost(http_request request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::FunctionCall call = RestServer::buildCallFromRequest(request);

        infra::Redis *redis = infra::Redis::getThreadConnection();

        redis->callFunction(call);

        if (call.isasync()) {
            // Don't wait for result
            logger->info("Async request ({}/{})", call.user(), call.function());
            request.reply(status_codes::Created, "Async request submitted\n");
        } else {
            logger->info("Sync request ({}/{})", call.user(), call.function());
            message::FunctionCall result = redis->getFunctionResult(call);
            request.reply(status_codes::OK, result.outputdata() + "\n");
            logger->info("Finished request ({}/{})", call.user(), call.function());
        }
    }

    void RestServer::handlePut(http_request request) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::FunctionCall call = RestServer::buildCallFromRequest(request);
        std::string outputFile = infra::getFunctionFile(call);

        logger->info("Uploading ({}/{})", call.user(), call.function());

        // Here the call input data is actually the file
        std::ofstream out(outputFile);
        const std::string &fileBody = call.inputdata();
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        wasm::WasmModule::compileToObjectFile(call);

        request.reply(status_codes::OK, "Upload complete\n");
    }

    message::FunctionCall RestServer::buildCallFromRequest(http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

        if(pathParts.size() != 3) {
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

