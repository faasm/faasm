#pragma once

#include <pistache/http.h>
#include <scheduler/SchedulerHttpMixin.h>

namespace knative {
    class KnativeHandler : public Pistache::Http::Handler, scheduler::SchedulerHttpMixin {
    public:
        HTTP_PROTOTYPE(KnativeHandler)

        void onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter writer) override;

        void onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) override;

        std::string handleFunction(const std::string &requestStr);
    };
}
