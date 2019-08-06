#pragma once

#include <http/HttpEndpoint.h>

using namespace Pistache;

namespace knative {
    class KnativeEndpoint : public http::HttpEndpoint {
    public:
        KnativeEndpoint();

        void setHandler();
    };
}
