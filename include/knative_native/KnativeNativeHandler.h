#pragma once

#include <scheduler/SchedulerHttpMixin.h>
#include <pistache/http.h>

namespace knative_native {
    class KnativeNativeHandler : public Pistache::Http::Handler, scheduler::SchedulerHttpMixin {
    public:
        KnativeNativeHandler(std::string userIn, std::string funcIn);

        HTTP_PROTOTYPE(KnativeNativeHandler)

        void onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) override;
    private:
        const std::string user;
        const std::string func;
        bool isCold = true;
    };

    void executePythonFunction();

    void setAsyncResult(const message::Message &msg);

}
