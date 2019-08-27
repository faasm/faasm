#include <wasm/WasmModule.h>
#include <util/config.h>
#include <util/timing.h>
#include <util/locks.h>
#include <runner/function.h>
#include <stdio.h>
#include <fstream>

#define USER "demo"
#define FUNCTION "noop"

#define TPT_LOG_FILE "/tmp/faasm_tpt.log"
#define LAT_LOG_FILE "/tmp/faasm_lat.log"

static std::mutex latFileMx;

void _execFunction() {
    const util::TimePoint &start = util::startTimer();

    runner::benchmarkExecutor(USER, FUNCTION);

    double elapsedMillis = util::getTimeDiffMillis(start);

    std::ofstream latFile;
    latFile.open(LAT_LOG_FILE, std::ios_base::app);

    // Write to latency file with lock
    util::UniqueLock lock(latFileMx);
    latFile << elapsedMillis << " LATENCY" << std::endl;
}


int main(int argc, char *argv[]) {
    // Set up conf
    util::SystemConfig &conf = util::getSystemConfig();
    conf.logLevel = "off";
    conf.cgroupMode = "on";
    conf.netNsMode = "off";
    conf.zygoteMode = "off";

    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide request delay (ms) and duration (ms)");
        return 1;
    }

    // Pre-flight
    _execFunction();

    // Get args
    int requestDelay = std::stoi(argv[1]);
    int duration = std::stoi(argv[2]);

    std::cout << "Running throughput bench with delay=" << requestDelay << "ms and duration=" << duration << "ms"
              << std::endl;

    // Set up files
    remove(LAT_LOG_FILE);
    remove(TPT_LOG_FILE);

    std::ofstream tptFile;
    tptFile.open(TPT_LOG_FILE, std::ios_base::app);

    const util::TimePoint &startTimer = util::startTimer();
    double elapsed = 0;
    int requestCount = 1;

    std::vector<std::thread> threads;

    while (elapsed < duration) {
        // Spawn execution in background
        threads.emplace_back(_execFunction);

        // Log to throughput file
        tptFile << elapsed << " REQUEST " << requestCount << std::endl;
        requestCount++;

        // Sleep
        usleep(requestDelay * 1000);

        // Update elapsed
        elapsed = util::getTimeDiffMillis(startTimer);
    }

    // Wait for any stragglers
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Finished";

    return 0;
}