#include "FunctionEndpoint.h"

#include <util/logging.h>

#include <pistache/http.h>
#include <pistache/router.h>

#include <scheduler/Scheduler.h>
#include <scheduler/GlobalMessageBus.h>

namespace edge {
    FunctionEndpoint::FunctionEndpoint() :
            Endpoint(8001, 40) {

        setupRoutes();
    }

    std::shared_ptr<Pistache::Http::Handler> FunctionEndpoint::getHandler() {
        return router.handler();
    }

    void FunctionEndpoint::setupRoutes() {
        // C/C++
        Pistache::Rest::Routes::Post(router, "/f/:user/:function",
                                     Pistache::Rest::Routes::bind(&FunctionEndpoint::handleFunctionWrapper, this));
        Pistache::Rest::Routes::Post(router, "/fa/:user/:function",
                                     Pistache::Rest::Routes::bind(&FunctionEndpoint::handleAsyncFunctionWrapper, this));

        // Python
        Pistache::Rest::Routes::Post(router, "/p/:user/:function",
                                     Pistache::Rest::Routes::bind(&FunctionEndpoint::handlePythonFunctionWrapper,
                                                                  this));
        Pistache::Rest::Routes::Post(router, "/pa/:user/:function",
                                     Pistache::Rest::Routes::bind(&FunctionEndpoint::handleAsyncPythonFunctionWrapper,
                                                                  this));

        // Typescript
        Pistache::Rest::Routes::Post(router, "/t/:user/:function",
                                     Pistache::Rest::Routes::bind(&FunctionEndpoint::handleTypescriptFunctionWrapper,
                                                                  this));
        Pistache::Rest::Routes::Post(router, "/ta/:user/:function",
                                     Pistache::Rest::Routes::bind(
                                             &FunctionEndpoint::handleAsyncTypescriptFunctionWrapper,
                                             this));
    }

    void FunctionEndpoint::handleFunctionWrapper(const Pistache::Rest::Request &request,
                                                 Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(false);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    void FunctionEndpoint::handleAsyncFunctionWrapper(const Pistache::Rest::Request &request,
                                                      Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(true);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    void FunctionEndpoint::handlePythonFunctionWrapper(const Pistache::Rest::Request &request,
                                                       Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(false);

        util::convertMessageToPython(msg);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    void
    FunctionEndpoint::handleAsyncPythonFunctionWrapper(const Pistache::Rest::Request &request,
                                                       Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(true);

        util::convertMessageToPython(msg);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    void FunctionEndpoint::handleTypescriptFunctionWrapper(const Pistache::Rest::Request &request,
                                                           Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(false);
        msg.set_istypescript(true);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    void FunctionEndpoint::handleAsyncTypescriptFunctionWrapper(const Pistache::Rest::Request &request,
                                                                Pistache::Http::ResponseWriter response) {
        message::Message msg = this->buildMessageFromRequest(request);
        msg.set_isasync(true);
        msg.set_istypescript(true);

        const std::string result = this->handleFunction(msg);
        response.send(Pistache::Http::Code::Ok, result);
    }

    std::string FunctionEndpoint::handleFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Make the call
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        globalBus.enqueueMessage(msg);

        const std::string funcStr = util::funcToString(msg, true);

        if (msg.isasync()) {
            logger->info("Async request {}", funcStr);
            return "Async request submitted";
        } else {
            util::SystemConfig &conf = util::getSystemConfig();
            logger->info("Sync request {}", funcStr);
            message::Message result = globalBus.getFunctionResult(msg.id(), conf.globalMessageTimeout);

            logger->info("Finished request {}", funcStr);

            return result.outputdata() + "\n";
        }
    }

    message::Message FunctionEndpoint::buildMessageFromRequest(const Pistache::Rest::Request &request) {
        // Parse request params
        auto user = request.param(":user").as<std::string>();
        auto function = request.param(":function").as<std::string>();

        // Get the request body
        const std::string requestData = request.body();

        // Build message
        message::Message msg = util::messageFactory(user, function);
        msg.set_inputdata(requestData);

        return msg;
    }
}

