#pragma once

#include <pistache/http.h>
#include <proto/faasm.pb.h>

namespace faaslet {
    class FaasletEndpointHandler : public Pistache::Http::Handler {
    public:
        HTTP_PROTOTYPE(FaasletEndpointHandler)

        void onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter writer) override;

        void onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) override;

        std::string handleFunction(const std::string &requestStr);

    private:
        std::string executeFunction(message::Message &msg);
    };
}
