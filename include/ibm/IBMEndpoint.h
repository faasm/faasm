#pragma once

#include <endpoint/Endpoint.h>

#include <pistache/http.h>
#include <pistache/router.h>
#include <worker/WorkerMain.h>


namespace ibm {
    class IBMEndpoint : public endpoint::Endpoint {
    public:
        IBMEndpoint();

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;

        void handleInit(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

        void handleCall(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

        void stop();
    private:
        Pistache::Rest::Router router;
        worker::WorkerMain *workerMain;

        void setupRoutes();

        std::string buildResponse(bool success, const std::string &msg);
    };
}
