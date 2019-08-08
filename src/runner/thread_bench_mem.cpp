#include <util/logging.h>
#include <unistd.h>

void _runFunction() {
    sleep(10);
}

int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->error("Must provide number of workers");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);

    // Spawn worker threads
    std::vector<std::thread> threads(nWorkers);
    for (int w = 0; w < nWorkers; w++) {
        logger->info("Starting worker thread {}", w);
        threads.emplace_back(std::thread(_runFunction));
    }

    // Rejoin
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}