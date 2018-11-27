#include "edge/edge.h"
#include "prof/prof.h"

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <infra/infra.h>
#include <util/util.h>

using namespace Pistache;

namespace edge {
    FunctionEndpoint::FunctionEndpoint() {
        int port = 8001;
        int threadCount = 40;

        Address addr(Ipv4::any(), Port(port));

        // Configure endpoint
        auto opts = Http::Endpoint::options()
                .threads(threadCount)
                .flags(Tcp::Options::InstallSignalHandler);

        httpEndpoint = std::make_shared<Http::Endpoint>(addr);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void FunctionEndpoint::start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void FunctionEndpoint::shutdown() {
        httpEndpoint->shutdown();
    }

    void FunctionEndpoint::setupRoutes() {
        using namespace Rest;

        Routes::Post(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::handleFunctionWrapper, this));
        Routes::Post(router, "/fa/:user/:function", Routes::bind(&FunctionEndpoint::handleAsyncFunctionWrapper, this));
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

    std::string FunctionEndpoint::handleFunction(message::Message &msg) {
        const std::chrono::steady_clock::time_point &t = prof::startTimer();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Bomb out if call isn't valid
        if (!infra::isValidFunction(msg)) {
            std::string errorMessage = infra::funcToString(msg) + " is not a valid function";
            return errorMessage;
        }

        // Make the call
        infra::Redis *redis = infra::Redis::getThreadConnection();
        redis->callFunction(msg);
        prof::logEndTimer("edge-submit", t);

        if (msg.isasync()) {
            logger->info("Async request {}", infra::funcToString(msg));
            return "Async request submitted";
        } else {
            logger->info("Sync request {}", infra::funcToString(msg));
            message::Message result = redis->getFunctionResult(msg);

            const std::chrono::steady_clock::time_point &t2 = prof::startTimer();

            logger->info("Finished request {}", infra::funcToString(msg));

            prof::logEndTimer("edge-reply", t2);

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
};

