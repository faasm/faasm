#pragma once

#include <proto/faasm.pb.h>
#include <scheduler/Scheduler.h>
#include <http/HttpEndpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>


namespace edge {
class FunctionEndpoint: public http::HttpEndpoint {
    public:
        FunctionEndpoint();

        std::string handleFunction(message::Message &msg);

        void setHandler();
    private:
        Rest::Router router;

        scheduler::GlobalMessageBus &globalBus;

        void setupRoutes();

        void handleFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response);

        void handleAsyncFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response);

        void handlePythonFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response);

        void handleAsyncPythonFunctionWrapper(const Rest::Request &request, Http::ResponseWriter response);

        message::Message buildMessageFromRequest(const Rest::Request &request);
    };
}