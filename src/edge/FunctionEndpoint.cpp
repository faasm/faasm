#include "FunctionEndpoint.h"

#include <signal.h>

#include <util/logging.h>

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <scheduler/Scheduler.h>
#include <scheduler/GlobalMessageBus.h>

using namespace Pistache;

namespace edge {
    FunctionEndpoint::FunctionEndpoint() : globalBus(scheduler::getGlobalMessageBus()) {
        int port = 8001;
        int threadCount = 40;

        Address addr(Ipv4::any(), Port(port));

        // Configure endpoint
        auto opts = Http::Endpoint::options()
                .threads(threadCount)
                .flags(Tcp::Options::ReuseAddr);

        httpEndpoint = std::make_shared<Http::Endpoint>(addr);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void FunctionEndpoint::start() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set up signal handler
        sigset_t signals;
        if (sigemptyset(&signals) != 0
            || sigaddset(&signals, SIGTERM) != 0
            || sigaddset(&signals, SIGINT) != 0
            || sigaddset(&signals, SIGHUP) != 0
            || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {

            throw std::runtime_error("Install signal handler failed");
        }

        // Configure and start endpoint
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();

        // Wait for a signal
        int signal = 0;
        int status = sigwait(&signals, &signal);
        if (status == 0) {
            logger->info("Received signal: {}", signal);
        } else {
            logger->info("Sigwait return value: {}", signal);
        }

        httpEndpoint->shutdown();
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
        util::addResultKeyToMessage(msg);
        globalBus.enqueueMessage(msg);

        if (msg.isasync()) {
            logger->info("Async request {}", util::funcToString(msg));
            return "Async request submitted";
        } else {
            logger->info("Sync request {}", util::funcToString(msg));
            message::Message result = globalBus.getFunctionResult(msg);

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
};

