#include "KnativeNativeHandler.h"

#include <emulator/emulator.h>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/bytes.h>
#include <util/json.h>
#include <util/func.h>
#include <util/environment.h>


namespace knative_native {
    KnativeNativeHandler::KnativeNativeHandler() = default;

    void KnativeNativeHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string funcName = util::getEnvVar("FAASM_FUNC", "");
        const std::string user = util::getEnvVar("FAASM_USER", "");

        if(funcName.empty() || user.empty()) {
            logger->error("Expected to find FAASM_FUNC and FAASM_USER environment variables");
            throw std::runtime_error("Need to set FAASM_USER and FAASM_FUNC");
        }

        // Set up what user/ function we're running
        logger->debug("Knative native request to {}/{}", user, funcName);
        setEmulatorUser(user.c_str());
        setEmulatorFunction(funcName.c_str());

        // Parse the JSON input
        const std::string requestStr = request.body();
        message::Message msg = util::jsonToMessage(requestStr);
        
        // Set the input to the function
        const std::vector<uint8_t> inputBytes = util::stringToBytes(msg.inputdata());
        setEmulatorInputData(inputBytes);
        setEmulatorFunctionIdx(msg.idx());

        // Invoke the native function
        logger->debug("Executing function index {}", msg.idx());
        exec(msg.idx());

        // Get the output
        std::string outputStr;
        const std::vector<uint8_t> outputData = getEmulatorOutputData();
        if(outputData.empty()) {
            outputStr = "Empty output";
        } else {
            outputStr = std::string(reinterpret_cast<const char*>(outputData.data()));
        }

        response.send(Pistache::Http::Code::Ok, outputStr);
    }
}
