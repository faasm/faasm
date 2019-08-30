#include <thread>
#include <vector>
#include <unistd.h>
#include <system_error>
#include <iostream>

#define SLEEP_SECONDS 2
#define N_THREADS 120000

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

    for (int i = 0; i < N_THREADS; i++) {
        bool success = _doThreadSpawn();
        if(!success) {
            std::cout << "Actual max threads = " << i << std::endl;
            break;
        }
    }

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}
