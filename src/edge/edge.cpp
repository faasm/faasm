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
        int threadCount = 20;

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
        message::FunctionCall call = this->buildCallFromRequest(request);
        call.set_isasync(false);

        const std::string result = this->handleFunction(call);
        response.send(Http::Code::Ok, result);
    }

    void FunctionEndpoint::handleAsyncFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response) {
        message::FunctionCall call = this->buildCallFromRequest(request);
        call.set_isasync(true);

        const std::string result = this->handleFunction(call);
        response.send(Http::Code::Ok, result);
    }

    std::string FunctionEndpoint::handleFunction(message::FunctionCall &call) {
        const std::chrono::steady_clock::time_point &t = prof::startTimer();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Make the call
        infra::Redis *redis = infra::Redis::getThreadConnection();
        redis->callFunction(call);
        prof::logEndTimer("edge-submit", t);

        if(call.isasync()) {
            logger->info("Async request {}", infra::funcToString(call));
            return "Async request submitted";
        }
        else {
            logger->info("Sync request {}", infra::funcToString(call));
            message::FunctionCall result = redis->getFunctionResult(call);

            const std::chrono::steady_clock::time_point &t2 = prof::startTimer();

            logger->info("Finished request {}", infra::funcToString(call));

            prof::logEndTimer("edge-reply", t2);

            return result.outputdata();
        }
    }

    message::FunctionCall FunctionEndpoint::buildCallFromRequest(const Rest::Request &request) {
        // Parse request params
        auto user = request.param(":user").as<std::string>();
        auto function = request.param(":function").as<std::string>();

        // Get the request body
        const std::string requestData = request.body();

        // Build function call
        message::FunctionCall call;
        call.set_user(user);
        call.set_function(function);
        call.set_inputdata(requestData);

        return call;
    }
};

