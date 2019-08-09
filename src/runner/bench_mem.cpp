#include "function.h"

#include <wasm/WasmModule.h>
#include <util/config.h>

#define USER "demo"
#define FUNCTION "sleep"

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->error("Must provide number of workers");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);

    logger->info("Running memory benchmark with {} workers", nWorkers);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";
    conf.cgroupMode = "on";

    // TODO - switch this on
    conf.netNsMode = "off";

    // Zygote mode increases memory footprint of each new sandbox
    conf.zygoteMode = "off";

    // Spawn worker threads to run the task in parallel
    std::vector<std::thread> threads(nWorkers);
    for (int w = 0; w < nWorkers; w++) {
        logger->info("Running worker {}", w);
        threads.emplace_back(std::thread(runner::benchmarkExecutor, USER, FUNCTION));
    }

    // Rejoin
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}