#include "function.h"

#include <wasm/WasmModule.h>
#include <util/config.h>

#define USER "demo"

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide function and number of workers");
        return 1;
    }

    // Get args
    std::string function(argv[1]);
    int nWorkers = std::stoi(argv[2]);

    logger->info("Running benchmark on demo/{} with {} workers", function, nWorkers);

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    // Zygote mode increases memory footprint of each new sandbox
    conf.zygoteMode = "off";

    // Make sure this is long enough to allow things to run their course
    conf.globalMessageTimeout = 120000;

    // Spawn worker threads to run the task in parallel
    std::vector<std::thread> threads(nWorkers);
    for (int w = 0; w < nWorkers; w++) {
        logger->info("Running worker {}", w);
        threads.emplace_back(std::thread(runner::benchmarkExecutor, USER, function));
    }

    // Wait for things to finish
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}