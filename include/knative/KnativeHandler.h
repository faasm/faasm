#pragma once

#include <pistache/http.h>
#include <scheduler/GlobalMessageBus.h>

namespace knative {
    class KnativeHandler : public Pistache::Http::Handler {
    public:
        KnativeHandler();

        HTTP_PROTOTYPE(KnativeHandler)

        void onTimeout(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter writer) override;

        void onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) override;

        std::string handleFunction(const std::string &requestStr);
    private:
        util::SystemConfig &conf;
    };
}
