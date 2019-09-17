#include <wasm/WasmModule.h>

#include <util/config.h>
#include <util/timing.h>
#include <runner/function.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 4) {
        logger->error("Must provide user, function name and run count");
        return 1;
    }

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    std::string user = argv[1];
    std::string function = argv[2];
    int runCount = std::stoi(argv[3]);

    for (int i = 0; i < runCount; i++) {
        logger->info("Run {} - {}/{} ", i, user, function);
        PROF_START(execution)

        runner::benchmarkExecutor(user, function);

        PROF_END(execution)
    }

    return 0;
}