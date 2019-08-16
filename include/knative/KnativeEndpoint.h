#pragma once

#include <http/HttpEndpoint.h>

namespace knative {
    class KnativeEndpoint : public http::HttpEndpoint {
    public:
        KnativeEndpoint();

        std::shared_ptr<Pistache::Http::Handler> getHandler() override;
    };
}
