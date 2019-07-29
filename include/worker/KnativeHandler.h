#pragma once

#include <pistache/http.h>

using namespace Pistache;

namespace worker {
    class KnativeHandler : public Http::Handler {
    public:
    HTTP_PROTOTYPE(KnativeHandler)

        void onRequest(const Http::Request &request, Http::ResponseWriter response);
    };
}
