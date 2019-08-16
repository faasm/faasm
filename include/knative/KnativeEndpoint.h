#pragma once

#include <http/HttpEndpoint.h>

namespace knative {
    class KnativeEndpoint : public http::HttpEndpoint {
    public:
        KnativeEndpoint();

        void setHandler();
    };
}
