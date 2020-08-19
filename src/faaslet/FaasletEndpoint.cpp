#include "FaasletEndpointHandler.h"
#include "FaasletEndpoint.h"

namespace faaslet {

    FaasletEndpoint::FaasletEndpoint() : Endpoint() {
    }

    FaasletEndpoint::FaasletEndpoint(int port, int threadCount) : Endpoint(port, threadCount) {
    }

    std::shared_ptr<Pistache::Http::Handler> FaasletEndpoint::getHandler() {
        return Pistache::Http::make_handler<FaasletEndpointHandler>();
    }

}
