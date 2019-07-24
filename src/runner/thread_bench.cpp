#include <util/logging.h>

void _doNoop(int nIterations) {
    // Have to do something here to avoid compiler optimising it away
    int *a = (int *) malloc((nIterations + 1) * sizeof(int));

    for (int i = 0; i < nIterations; i++) {
        a[i] = a[i + 1];
        a[i + 1] = i;
    }

    free(a);
}


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide number of workers and iterations");
        return 1;
    }

    // Get args
    int nWorkers = std::stoi(argv[1]);
    int nIterations = std::stoi(argv[2]);

    // Spawn worker threads
    std::vector<std::thread> threads(nWorkers);
    for (int w = 0; w < nWorkers; w++) {
        logger->info("Spawning native thread {}", w);
        threads.emplace_back(std::thread([nIterations] {
            _doNoop(nIterations);
        }));
    }

    // Rejoin
    for(auto &t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    return 0;
}