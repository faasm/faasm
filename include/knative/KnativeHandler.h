#pragma once

#include <pistache/http.h>

using namespace Pistache;

namespace knative {
    class KnativeHandler final : public Http::Handler {
    public:
        HTTP_PROTOTYPE(KnativeHandler)

        void onRequest(const Http::Request &request, Http::ResponseWriter response) override;
    };
}
