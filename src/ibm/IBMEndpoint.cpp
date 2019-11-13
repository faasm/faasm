#include "IBMEndpoint.h"

#include <util/logging.h>
#include <util/timing.h>

#include <scheduler/Scheduler.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <util/json.h>
#include <storage/FileLoader.h>


namespace ibm {
    std::string getStringFromJson(rapidjson::Document &d, const std::string &key) {
        rapidjson::Value::MemberIterator it = d["value"].FindMember(key.c_str());
        if (it == d["value"].MemberEnd()) {
            throw util::JsonFieldNotFound("Key not found: " + key);
        } else if (it->value.IsNull()) {
            throw util::JsonFieldNotFound("Key is null: " + key);
        } else {
            std::string strVal(it->value.GetString(), it->value.GetString() + it->value.GetStringLength());
            if (strVal.empty()) {
                throw util::JsonFieldNotFound("Value is empty: " + key);
            }

            return strVal;
        }
    }

    void setConfPropertyFromJson(rapidjson::Document &d, const std::string &key, std::string &property) {
        std::string value;

        try {
            value = getStringFromJson(d, key);
        } catch (util::JsonFieldNotFound &ex) {
            util::getLogger()->warn("Did not get parameter {} although expected it", key);
            return;
        }

        property = value;
    }

    IBMEndpoint::IBMEndpoint() : Endpoint(8080, 8) {
        setupRoutes();
    }

    std::shared_ptr<Pistache::Http::Handler> IBMEndpoint::getHandler() {
        return router.handler();
    }

    void IBMEndpoint::stop() {
        workerMain->shutdown();

        delete workerMain;
    }

    void IBMEndpoint::setupRoutes() {
        Pistache::Rest::Routes::Post(router, "/init",
                                     Pistache::Rest::Routes::bind(&IBMEndpoint::handleInit, this));

        Pistache::Rest::Routes::Post(router, "/run",
                                     Pistache::Rest::Routes::bind(&IBMEndpoint::handleCall, this));

    }

    void IBMEndpoint::handleInit(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
        const std::string requestStr = request.body();

        // Return a response
        const std::string responseStr = doHandleInit(requestStr);
        response.send(Pistache::Http::Code::Ok, responseStr);
    }

    std::string IBMEndpoint::doHandleInit(const std::string &requestStr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("INIT REQUEST - {}", requestStr);

        // Set up the config
        util::SystemConfig &conf = util::getSystemConfig();
        conf.cgroupMode = "off";
        conf.netNsMode = "off";
        conf.functionStorage = "ibm";
        conf.bucketName = "faasm-runtime";

        // Return a response
        const std::string &responseStr = buildResponse(true, "Initialised");
        return responseStr;
    }

    void IBMEndpoint::handleCall(const Pistache::Rest::Request &request, Pistache::Http::ResponseWriter response) {
        PROF_START(ibmRoundTrip)

        const std::string requestStr = request.body();
        const std::string responseStr = doHandleCall(requestStr);

        PROF_END(ibmRoundTrip)
        response.send(Pistache::Http::Code::Ok, responseStr);
    }

    std::string IBMEndpoint::doHandleCall(const std::string &requestStr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("CALL REQUEST - {}", requestStr);

        rapidjson::Document requestJson;
        requestJson.Parse(requestStr.c_str());

        std::string callMode;
        try {
            callMode = getStringFromJson(requestJson, "mode");
        } catch (util::JsonFieldNotFound &ex) {
            callMode = "";
        }

        // Start up the worker if not started already
        if (!started && !testMode) {
            util::UniqueLock lock(startedMutex);
            if (!started) {
                logger->info("Starting worker pool");

                // Set up the environment
                util::SystemConfig &conf = util::getSystemConfig();
                setConfPropertyFromJson(requestJson, "IBM_API_KEY", conf.ibmApiKey);
                setConfPropertyFromJson(requestJson, "REDIS_QUEUE_HOST", conf.redisQueueHost);
                setConfPropertyFromJson(requestJson, "REDIS_STATE_HOST", conf.redisStateHost);

                conf.print();

                workerMain = new worker::WorkerMain();
                workerMain->startBackground();

                started = true;
            }
        }

        std::string responseMsg;
        message::Message msg;

        try {
            msg.set_user(getStringFromJson(requestJson, "user"));
            msg.set_function(getStringFromJson(requestJson, "function"));
        } catch (util::JsonFieldNotFound &ex) {
            responseMsg = "User and function must be present in request";
        }

        try {
            msg.set_inputdata(getStringFromJson(requestJson, "input"));
        } catch (util::JsonFieldNotFound &ex) {
            // Fine to have no input
        }

        // Check for async
        rapidjson::Value::MemberIterator it = requestJson["value"].FindMember("async");
        if (it != requestJson["value"].MemberEnd()) {
            msg.set_isasync(it->value.GetBool());
        }

        // Do the actual execution
        if (responseMsg.empty()) {
            if (callMode == CODEGEN_MODE) {
                logger->info("Handling codegen {}", util::funcToString(msg, false));
                storage::FileLoader &loader = storage::getFileLoader();

                try {
                    loader.codegenForFunction(msg);
                    responseMsg = "Codegen successful";
                } catch (std::runtime_error &ex) {
                    responseMsg = "Failed running codegen";
                }

            } else if (callMode == INVOKE_MODE || callMode.empty()) {
                responseMsg = executeFunction(msg);
            } else {
                responseMsg = "Invalid call mode: " + callMode;
            }
        }

        const std::string &responseStr = buildResponse(true, responseMsg);
        return responseStr;
    }

    std::string IBMEndpoint::buildResponse(bool success, const std::string &msg) {
        rapidjson::Document d;
        d.SetObject();

        int statusCode = 200;
        if (!success) {
            statusCode = 500;
            d.AddMember("error", rapidjson::StringRef(msg.c_str()), d.GetAllocator());
        } else {
            d.AddMember("result", rapidjson::StringRef(msg.c_str()), d.GetAllocator());
        }

        d.AddMember("status_code", statusCode, d.GetAllocator());

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        d.Accept(writer);

        return sb.GetString();
    }
}
