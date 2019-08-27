#include <wasm/WasmModule.h>
#include <util/config.h>
#include <runner/function.h>

#define USER "demo"
#define FUNCTION "noop"


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->error("Must provide number iterations");
        return 1;
    }

    // Get args
    int nIterations = std::stoi(argv[1]);

    logger->info("Running Faasm benchmark with {} iterations", nIterations);

    // For each iteration we want to spawn a thread and execute the function
    // (to introduce the overhead we'd see in the real application)
    for (int i = 0; i < nIterations; i++) {
        logger->info("Running iteration {}", i);

        std::thread t(runner::benchmarkExecutor, USER, FUNCTION);
        t.join();
    }

    return 0;
}