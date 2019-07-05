#include <wasm/WasmModule.h>

#include <util/config.h>
#include <runner/timing.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    std::string user = argv[1];
    std::string function = argv[2];

    message::Message call;
    call.set_user(user);
    call.set_function(function);

    logger->info("Running function {}/{}", user, function);

    if (argc > 3) {
        std::string inputData = argv[3];
        call.set_inputdata(inputData);

        logger->info("Adding input data: {}", inputData);
    }

    const util::TimePoint tpInit = runner::startTimer();
    wasm::WasmModule module;
    module.initialise();
    module.bindToFunction(call);
    runner::logEndTimer("WASM initialisation", tpInit);

    const util::TimePoint tpSnap = runner::startTimer();
    module.snapshotFullMemory("nothing");
    runner::logEndTimer("WASM snapshot", tpSnap);

    const util::TimePoint tp = runner::startTimer();
    module.execute(call);
    runner::logEndTimer("WASM function execution", tp);
}