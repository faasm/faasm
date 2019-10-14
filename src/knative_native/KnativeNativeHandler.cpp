#include "KnativeNativeHandler.h"

#include <emulator/emulator.h>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/bytes.h>


namespace knative_native {
    KnativeNativeHandler::KnativeNativeHandler() = default;

    void KnativeNativeHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("KnativeNative handler received request");

        const std::string requestStr = request.body();
        logger->debug("Input = {}", requestStr);
        const std::vector<uint8_t> inputBytes = util::stringToBytes(requestStr);
        setEmulatorInputData(inputBytes);

        // Invoke the native function
        exec(0);

        // Get the output
        const std::vector<uint8_t> outputData = getEmulatorOutputData();
        auto outputStr = reinterpret_cast<const char*>(outputData.data());

        response.send(Pistache::Http::Code::Ok, outputStr);
    }
}
