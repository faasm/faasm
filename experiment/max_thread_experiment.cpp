#include <thread>
#include <vector>
#include <unistd.h>

int main() {
    int nThreads = 10000;
    std::vector<std::thread> threads;
    threads.reserve(nThreads);

    int seconds = 30;

    for(int i = 0 ; i < nThreads; i++) {
        threads.emplace_back([seconds] {
            sleep(seconds);
        });
    }
}
