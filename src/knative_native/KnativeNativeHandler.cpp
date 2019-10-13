#include "KnativeNativeHandler.h"

#include <faasm/core.h>
#include <emulator/emulator.h>

#include <util/logging.h>
#include <util/timing.h>


namespace knative_native {
    KnativeNativeHandler::KnativeNativeHandler() = default;

    void KnativeNativeHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("KnativeNative handler received request");

        PROF_START(knativeNativeRoundTrip)

        // TODO - set input from request data

        // Invoke the native function
        exec(0);

        // Get the output
        const std::vector<uint8_t> outputData = getEmulatorOutputData();
        auto outputStr = reinterpret_cast<const char*>(outputData.data());

        PROF_END(knativeNativeRoundTrip)
        response.send(Pistache::Http::Code::Ok, outputStr);
    }
}
