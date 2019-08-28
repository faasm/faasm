#include <thread>
#include <vector>
#include <unistd.h>
#include <system_error>
#include <iostream>

#define SLEEP_SECONDS 30
#define N_THREADS 50000

static std::vector<std::thread> threads;

bool _doThreadSpawn() {
    try {
        threads.emplace_back([] {
            sleep(SLEEP_SECONDS);
        });
    } catch (std::system_error &e) {
        std::cerr << "What: " << e.what() << "  Code: " << e.code() << std::endl;
        return false;
    }

    return true;
}

int main() {
    threads.reserve(N_THREADS);

    int failureCount = 0;
    bool totalFailure = false;

    for (int i = 0; i < N_THREADS; i++) {
        bool success = _doThreadSpawn();

        // Try again a few times
        if (!success) {
            std::cout << i << ": sleep 1" << std::endl;
            usleep(50);
            success = _doThreadSpawn();
        }

        if (!success) {
            std::cout << i << ": sleep 2" << std::endl;
            usleep(1000);
            success = _doThreadSpawn();
        }

        if (!success) {
            std::cout << i << ": sleep 3" << std::endl;
            usleep(10000);
            _doThreadSpawn();
        }

        if (!success) {
            std::cerr << "Failed to spawn with three retries" << std::endl;
            totalFailure = true;
            break;
        }
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    if(totalFailure) {
        return 1;
    }

    std::cout << "Successful threads = " << threads.size() << std::endl;

    std::cout << "Finished with " << failureCount << " failures out of " << N_THREADS << std::endl;
}
