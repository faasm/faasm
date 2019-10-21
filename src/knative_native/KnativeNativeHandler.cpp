#include "KnativeNativeHandler.h"

#include <emulator/emulator.h>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/bytes.h>
#include <util/json.h>
#include <util/environment.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>


namespace knative_native {
    KnativeNativeHandler::KnativeNativeHandler() = default;

    void KnativeNativeHandler::onRequest(
            const Pistache::Http::Request &request,
            Pistache::Http::ResponseWriter response
            ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string funcName = util::getEnvVar("FAASM_FUNC", "");
        const std::string user = util::getEnvVar("FAASM_USER", "");

        if (funcName.empty() || user.empty()) {
            logger->error("Expected to find FAASM_FUNC and FAASM_USER environment variables");
            throw std::runtime_error("Need to set FAASM_USER and FAASM_FUNC");
        }

        // Set up what user/ function we're running
        logger->debug("Knative native request to {}/{}", user, funcName);
        setEmulatorUser(user.c_str());
        setEmulatorFunction(funcName.c_str());

        // Parse the JSON input
        const std::string requestStr = request.body();
        logger->debug("Knative native request: {}", requestStr);

        message::Message msg = util::jsonToMessage(requestStr);
        util::setMessageId(msg);
        setEmulatorFunctionIdx(msg.idx());

        // Set the input to the function
        logger->debug("Knative native input: {}", msg.inputdata());
        const std::vector<uint8_t> inputBytes = util::stringToBytes(msg.inputdata());
        setEmulatorInputData(inputBytes);

        std::string outputStr;
        if (msg.isstatusrequest()) {
            // Message status request
            logger->debug("Getting status for function {}", msg.id());
            outputStr = getMessageStatus(msg);
        }
        else if(msg.ispython()) {
            setEmulatorPythonUser(msg.pythonuser().c_str());
            setEmulatorPythonFunction(msg.pythonfunction().c_str());

            logger->debug("Executing Python function {}/{}", msg.pythonuser(), msg.pythonfunction());

            // This will be linked to the python executable
            exec(0);
        }
        else if (msg.isasync()) {
            logger->debug("Executing function index {} async", msg.idx());

            // Execute async message in detached thread
            std::thread t([msg] {
                exec(msg.idx());

                // Set result of request
                scheduler::GlobalMessageBus &messageBus = scheduler::getGlobalMessageBus();
                message::Message resultMsg = msg;
                messageBus.setFunctionResult(resultMsg, true);
            });

            t.detach();

            outputStr = util::buildAsyncResponse(msg);
        } else {
            logger->debug("Executing function index {} sync", msg.idx());
            exec(msg.idx());

            // Get the output
            const std::vector<uint8_t> outputData = getEmulatorOutputData();
            if (outputData.empty()) {
                outputStr = "Empty output";
            } else {
                outputStr = std::string(reinterpret_cast<const char *>(outputData.data()));
            }
        }

        // Make sure we flush stdout
        fflush(stdout);

        response.send(Pistache::Http::Code::Ok, outputStr);
    }
}
