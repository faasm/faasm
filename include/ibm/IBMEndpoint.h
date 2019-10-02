#pragma once

#include <endpoint/Endpoint.h>

#include <pistache/http.h>
#include <pistache/router.h>


namespace ibm {
    class IBMEndpoint : public endpoint::Endpoint {
    public:
        IBMEndpoint();

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    private:
        Pistache::Rest::Router router;

        void setupRoutes();

        void handleInit(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

        void handleCall(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response);

        std::string buildResponse(bool success, const std::string &msg);
    };
}
