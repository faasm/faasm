#include "IBMEndpoint.h"

#include <util/logging.h>
#include <util/timing.h>

#include <scheduler/Scheduler.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace ibm {
    IBMEndpoint::IBMEndpoint() : Endpoint(8080, 8) {
        setupRoutes();
    }

    std::shared_ptr<Pistache::Http::Handler> IBMEndpoint::getHandler() {
        return router.handler();
    }

    void IBMEndpoint::setupRoutes() {
        Pistache::Rest::Routes::Post(router, "/init",
                                     Pistache::Rest::Routes::bind(&IBMEndpoint::handleInit, this));

        Pistache::Rest::Routes::Post(router, "/run",
                                     Pistache::Rest::Routes::bind(&IBMEndpoint::handleCall, this));

    }

    void IBMEndpoint::handleInit(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {

    }

    void IBMEndpoint::handleCall(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {

    }

    std::string IBMEndpoint::buildResponse(bool success, const std::string &msg) {
        rapidjson::Document d;
        d.SetObject();

        int statusCode = 200;
        if (!success) {
            statusCode = 500;
        }

        d.AddMember("status_code", statusCode, d.GetAllocator());

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        d.Accept(writer);

        return sb.GetString();
    }
}
