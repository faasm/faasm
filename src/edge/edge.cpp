#include "edge/edge.h"

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <infra/infra.h>
#include <util/util.h>

using namespace Pistache;

/**
 * HTTP endpoint for managing function calls.
 */
namespace edge {
    FunctionEndpoint::FunctionEndpoint(Address addr) :
            httpEndpoint(std::make_shared<Http::Endpoint>(addr)),
            redis(std::make_shared<infra::RedisClient>()) {}

    /**
     * Configures the endpoint (including threading) and its routing
     */
    void FunctionEndpoint::init(int threadCount) {
        // Configure endpoint
        auto opts = Http::Endpoint::options()
                .threads(threadCount)
                .flags(Tcp::Options::InstallSignalHandler);

        httpEndpoint->init(opts);
        setupRoutes();

        // Connect to redis
        redis->connect();
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

        Routes::Put(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::uploadFunction, this));
        Routes::Post(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::handleFunction, this));
        Routes::Post(router, "/fa/:user/:function", Routes::bind(&FunctionEndpoint::handleAsyncFunction, this));
        Routes::Get(router, "/status/", Routes::bind(&FunctionEndpoint::status, this));
    }

    void FunctionEndpoint::handleFunction(const Rest::Request &request, Http::ResponseWriter response) {
        message::FunctionCall call = this->buildCallFromRequest(request);

        // Make the call
        redis->callFunction(call);

        // Wait for the result
        std::cout << "Awaiting result " << call.user() << " - " << call.function() << "\n";
        message::FunctionCall result = redis->blockingGetFunctionResult(call);

        // Handle result
        std::cout << "Result " << result.user() << " - " << result.function() << "\n";
        if (result.success()) {
            response.send(Http::Code::Ok, result.outputdata());
        } else {
            response.send(Http::Code::Internal_Server_Error, "Error");
        }
    }

    void FunctionEndpoint::handleAsyncFunction(const Rest::Request &request, Http::ResponseWriter response) {
        message::FunctionCall call = this->buildCallFromRequest(request);

        // Make the call
        redis->callFunction(call);

        // Don't wait for a result
        response.send(Http::Code::Ok, "Async success");
    }

    void FunctionEndpoint::uploadFunction(const Rest::Request &request, Http::ResponseWriter response) {
        message::FunctionCall call = this->buildCallFromRequest(request);

        // Make the call
        redis->callFunction(call);

        // Don't wait for a result
        response.send(Http::Code::Ok, "Async success");
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

    void FunctionEndpoint::status(const Rest::Request &request, Http::ResponseWriter response) {
        std::string statusString = "Ok";
        std::string redisCheck = redis->check(statusString);
        response.send(Http::Code::Ok, redisCheck);
    }

};

