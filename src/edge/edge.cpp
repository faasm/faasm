#include "edge/edge.h"

#include <infra/infra.h>
#include <util/util.h>

namespace edge {

    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redis;

    RestServer::RestServer() {

    }

    void RestServer::listen(const std::string &port) {
        std::string addr = "http://0.0.0.0:" + port;
        http_listener listener(addr);

        listener.support(methods::GET, RestServer::handleGet);

        listener.support(methods::POST, RestServer::handlePost);

        listener.support(methods::PUT, RestServer::handlePut);

        listener.open().then([port]() {
            std::cout << "Listening for requests on localhost:" << port << std::endl;
        }).wait();

        // Continuous loop...
        while (true);
    }

    void RestServer::handleGet(http_request request) {
        request.reply(status_codes::OK, "Get OK");
    }

    void RestServer::handlePost(http_request request) {
        message::FunctionCall call = RestServer::buildCallFromRequest(request);

        redis.callFunction(call);

        if (call.isasync()) {
            // Don't wait for result
            std::cout << "Submitted async " << call.user() << " - " << call.function() << std::endl;
            request.reply(status_codes::Created, "Async request submitted");
        } else {
            std::cout << "Awaiting result for " << call.user() << " - " << call.function() << std::endl;

            message::FunctionCall result = redis.getFunctionResult(call);

            if (result.success()) {
                request.reply(status_codes::OK, result.outputdata() + "\n");
            } else {
                request.reply(status_codes::InternalError, "Error");
            }
        }
    }

    void RestServer::handlePut(http_request request) {
        request.reply(status_codes::OK, "Put OK");
    }

    message::FunctionCall RestServer::buildCallFromRequest(http_request &request) {
        const uri uri = request.relative_uri();
        const std::vector<std::string> pathParts = uri::split_path(uri::decode(uri.path()));

        // Check URI
        if (pathParts[0] == "f" || pathParts[0] == "fa") {
            message::FunctionCall call;
            call.set_user(pathParts[1]);
            call.set_function(pathParts[2]);

            // Read in request body
            request.extract_string().then([&](pplx::task<std::string> task) {
                auto requestData = task.get();

                if(requestData.empty()) {
                    call.set_inputdata(requestData);
                }

            }).wait();

            call.set_isasync(pathParts[0] == "fa");

            return call;
        }

        throw InvalidPathException();
    }

};

