#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

using namespace Pistache;

/**
 * HTTP endpoint for managing function calls.
 */
class FunctionEndpoint {

public:
    FunctionEndpoint(Address addr) :
            httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    /**
     * Configures the endpoint (including threading) and its routing
     */
    void init(int threadCount) {
        // Configure endpoint
        auto opts = Http::Endpoint::options()
                .threads(threadCount)
                .flags(Tcp::Options::InstallSignalHandler);

        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() {
        httpEndpoint->shutdown();
    }

private:
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;

    void setupRoutes() {
        using namespace Rest;

        Routes::Post(router, "/f/:user/:function", Routes::bind(&FunctionEndpoint::function, this));
        Routes::Get(router, "/status/", Routes::bind(&FunctionEndpoint::status, this));
    }

    void function(const Rest::Request &request, Http::ResponseWriter response) {
        auto user = request.param(":user").as<std::string>();
        auto function = request.param(":function").as<std::string>();

        bool success = false;

        //TODO Put request on queue

        //TODO Wait for response

        if (success) {
            response.send(Http::Code::Ok, "Success");
        } else {
            response.send(Http::Code::Internal_Server_Error, "Error");
        }
    }

    void status(const Rest::Request &request, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Healthy \n");
    }
};

int main() {
    Port port(8080);

    int threadCount = 4;

    Address addr(Ipv4::any(), port);

    FunctionEndpoint endpoint(addr);

    endpoint.init(threadCount);
    endpoint.start();

    endpoint.shutdown();
}
