#pragma once

#include <pistache/http.h>
#include <scheduler/GlobalMessageBus.h>

using namespace Pistache;

namespace knative {
    class KnativeHandler final : public Http::Handler {
    public:
        KnativeHandler();

        HTTP_PROTOTYPE(KnativeHandler)

        void onRequest(const Http::Request &request, Http::ResponseWriter response) override;
    private:
        scheduler::GlobalMessageBus &globalBus;
        util::SystemConfig &conf;
    };
}
