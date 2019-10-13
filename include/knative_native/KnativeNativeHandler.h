#pragma once

#include <pistache/http.h>

namespace knative_native {
    class KnativeNativeHandler : public Pistache::Http::Handler {
    public:
        KnativeNativeHandler();

        HTTP_PROTOTYPE(KnativeNativeHandler)

        void onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) override;
    };
}
