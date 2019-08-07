#include <util/logging.h>

void _runFunction() {
    int n = 4;
    // Have to do something here to avoid compiler optimising it away
    int *a = (int *) malloc((n + 1) * sizeof(int));

    for (int i = 0; i < n; i++) {
        a[i] = a[i + 1];
        a[i + 1] = i;
    }

    free(a);
}


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 2) {
        logger->error("Must provide number of iterations");
        return 1;
    }

    // Get args
    int nIterations = std::stoi(argv[1]);
    for (int i = 0; i < nIterations; i++) {
        logger->info("Running iteration {}", i);
        std::thread t(_runFunction);
        t.join();
    }

    return 0;
}