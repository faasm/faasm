#pragma once

#include <proto/faasm.pb.h>
#include <scheduler/Scheduler.h>
#include <endpoint/Endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>


namespace edge {
    class FunctionEndpoint : public endpoint::Endpoint {
    public:
        FunctionEndpoint();

        std::string handleFunction(message::Message &msg);

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;

    private:
        Pistache::Rest::Router router;

        scheduler::GlobalMessageBus &globalBus;

        void setupRoutes();

        void handleFunctionWrapper(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

        void handleAsyncFunctionWrapper(const Pistache::Rest::Request &request,
                                        Pistache::Http::ResponseWriter response);

        void handlePythonFunctionWrapper(const Pistache::Rest::Request &request,
                                         Pistache::Http::ResponseWriter response);

        void handleAsyncPythonFunctionWrapper(const Pistache::Rest::Request &request,
                                              Pistache::Http::ResponseWriter response);

        message::Message buildMessageFromRequest(const Pistache::Rest::Request &request);
    };
}