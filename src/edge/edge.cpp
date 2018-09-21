#include "edge/edge.h"


namespace edge {

    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redis;

    RestServer::RestServer() = default;

    void RestServer::listen(const std::string &port) {
        std::string addr = "http://0.0.0.0:" + port;
        http_listener listener(addr);

        listener.support(methods::GET, RestServer::handleGet);

        listener.support(methods::POST, RestServer::handlePost);

        listener.support(methods::PUT, RestServer::handlePut);

        listener.open().wait();

        // Continuous loop required to allow listening apparently
        while (true) {
            std::cout << "Listening for requests on localhost:" << port << std::endl;
            usleep(60 * 1000 * 1000);
        }
    }

    void RestServer::handleGet(http_request request) {
        request.reply(status_codes::OK, "Get OK");
    }

    void RestServer::handlePost(http_request request) {
        message::FunctionCall call = RestServer::buildCallFromRequest(request);

        redis.callFunction(call);

        if (call.isasync()) {
            // Don't wait for result
            std::cout << "Submitted async " << call.user() << " - " << call.function() << " - " << call.inputdata()
                      << std::endl;
            request.reply(status_codes::Created, "Async request submitted\n");
        } else {
            std::cout << "Awaiting result for " << call.user() << " - " << call.function() << " - " << call.inputdata()
                      << std::endl;

            message::FunctionCall result = redis.getFunctionResult(call);

            if (result.success()) {
                request.reply(status_codes::OK, result.outputdata() + "\n");
            } else {
                request.reply(status_codes::InternalError, "Error\n");
            }
        }
    }

    void RestServer::handlePut(http_request request) {
        message::FunctionCall call = RestServer::buildCallFromRequest(request);
        std::string outputFile = infra::getFunctionFile(call);

        std::cout << "Uploading " << call.user() << " - " << call.function() << " to " << outputFile << std::endl;

        // Here the call input data is actually the file
        std::ofstream out(outputFile);
        const std::string &fileBody = call.inputdata();
        out.write(fileBody.c_str(), fileBody.size());
        out.flush();
        out.close();

        // Build the object file from the file we've just received
        std::vector<uint8_t> objBytes = wasm::WasmModule::compile(call);
        std::string objFilePath = infra::getFunctionObjectFile(call);
        util::writeBytesToFile(objFilePath, objBytes);

        request.reply(status_codes::OK, "Upload complete\n");
    }

    message::FunctionCall RestServer::buildCallFromRequest(http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

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

