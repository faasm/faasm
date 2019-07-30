#include "FunctionEndpoint.h"

#include <util/logging.h>

#include <pistache/http.h>
#include <pistache/router.h>

#include <scheduler/Scheduler.h>
#include <scheduler/GlobalMessageBus.h>

using namespace Pistache;

namespace edge {
    FunctionEndpoint::FunctionEndpoint() :
            HttpEndpoint(8001, 40),
            globalBus(scheduler::getGlobalMessageBus()) {

        setupRoutes();
    }

    void FunctionEndpoint::setHandler() {
        httpEndpoint->setHandler(router.handler());
    }

    void FunctionEndpoint::setupRoutes() {
        using namespace Rest;

        Routes::Post(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::handleFunctionWrapper, this));
        Routes::Post(router, "/fa/:user/:function", Routes::bind(&FunctionEndpoint::handleAsyncFunctionWrapper, this));
        Routes::Post(router, "/p/:user/:function", Routes::bind(&FunctionEndpoint::handlePythonFunctionWrapper, this));
        Routes::Post(router, "/pa/:user/:function",
                     Routes::bind(&FunctionEndpoint::handleAsyncPythonFunctionWrapper, this));
    }

    void FunctionEndpoint::handleFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(false);

        const std::string result = this->handleFunction(msg);
        response.send(Http::Code::Ok, result);
    }

    void FunctionEndpoint::handleAsyncFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(true);

        const std::string result = this->handleFunction(msg);
        response.send(Http::Code::Ok, result);
    }

    void FunctionEndpoint::handlePythonFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(false);
        msg.set_ispython(true);

        const std::string result = this->handleFunction(msg);
        response.send(Http::Code::Ok, result);
    }

    void
    FunctionEndpoint::handleAsyncPythonFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(true);
        msg.set_ispython(true);

        const std::string result = this->handleFunction(msg);
        response.send(Http::Code::Ok, result);
    }

    std::string FunctionEndpoint::handleFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Make the call
        util::setMessageId(msg);
        globalBus.enqueueMessage(msg);

        if (msg.isasync()) {
            logger->info("Async request {}", util::funcToString(msg));
            return "Async request submitted";
        } else {
            logger->info("Sync request {}", util::funcToString(msg));
            message::Message result = globalBus.getFunctionResult(msg.id());

            logger->info("Finished request {}", util::funcToString(msg));

            return result.outputdata() + "\n";
        }
    }

    message::Message FunctionEndpoint::buildMessageFromRequest(const Rest::Request &request) {
        // Parse request params
        auto user = request.param(":user").as<std::string>();
        auto function = request.param(":function").as<std::string>();

        // Get the request body
        const std::string requestData = request.body();

        // Build message
        message::Message msg;
        msg.set_user(user);
        msg.set_function(function);
        msg.set_inputdata(requestData);

        return msg;
    }
}

