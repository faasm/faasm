#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <redis/redis.h>
#include "edge/edge.h"
#include <proto/faasm.pb.h>

using namespace Pistache;

/**
 * HTTP endpoint for managing function calls.
 */
namespace edge {
    FunctionEndpoint::FunctionEndpoint(Address addr) :
            httpEndpoint(std::make_shared<Http::Endpoint>(addr)),
            redis(std::make_shared<redis::RedisClient>()) {}

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

        Routes::Post(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::handleFunction, this));
        Routes::Get(router, "/status/", Routes::bind(&FunctionEndpoint::status, this));
    }

    void FunctionEndpoint::handleFunction(const Rest::Request &request, Http::ResponseWriter response) {
        auto user = request.param(":user").as<std::string>();
        auto function = request.param(":function").as<std::string>();

        bool success = true;

        message::FunctionCall call;
        call.set_user(user);
        call.set_function(function);

        redis->callFunction(call);

        //TODO wait for/ handle response

        // Note, send is async
        if (success) {
            response.send(Http::Code::Ok, "Success");
        } else {
            response.send(Http::Code::Internal_Server_Error, "Error");
        }
    }

    void FunctionEndpoint::status(const Rest::Request &request, Http::ResponseWriter response) {
        std::string statusString = "Ok";
        std::string redisCheck = redis->check(statusString);
        response.send(Http::Code::Ok, redisCheck);
    }
};

