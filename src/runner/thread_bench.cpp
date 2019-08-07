#include <util/logging.h>
#include <unistd.h>
#include <stdio.h>

void _runFunction(bool addSleep) {
    if (addSleep) {
        sleep(10);
    } else {
        int n = 4;
        // Have to do something here to avoid compiler optimising it away
        int *a = (int *) malloc((n + 1) * sizeof(int));

        for (int i = 0; i < n; i++) {
            a[i] = a[i + 1];
            a[i + 1] = i;
        }

        free(a);
    }
}


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 4) {
        logger->error("Must provide number of workers, iterations and flag whether to sleep");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);
    int nIterations = std::stoi(argv[2]);
    bool addSleep = std::stoi(argv[3]) > 0;

    if (nWorkers > 1) {
        if (nIterations == 1 || !addSleep) {
            logger->error("Expected multiple workers running a single sleep");
            return 1;
        }

        // Spawn worker threads
        std::vector<std::thread> threads(nWorkers);
        for (int w = 0; w < nWorkers; w++) {
            logger->info("Starting worker thread {}", w);
            threads.emplace_back(std::thread([addSleep] {
                _runFunction(addSleep);
            }));
        }

        // Rejoin
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } else {
        if (nWorkers != 1 || addSleep) {
            logger->error("Expected one worker running multiple iterations");
            return 1;
        }

        for (int i = 0; i < nIterations; i++) {
            logger->info("Running iteration {}", i);
            std::thread t([addSleep] {
                _runFunction(addSleep);
            });

            t.join();
        }
    }

    return 0;
}