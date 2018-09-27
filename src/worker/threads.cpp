#include "worker.h"

#include <algorithm>

namespace worker {

    static std::mutex threadsMutex;

    ThreadPool::ThreadPool(int nThreads) : nThreads(nThreads) {
        for (int i = 0; i < nThreads; i++) {
            this->allThreads.insert(i);
        }
    };

    int ThreadPool::getSetExecuting() {
        std::lock_guard<std::mutex> guard(threadsMutex);

        int availableIdx = this->getAvailable();

        this->executingThreads.insert(availableIdx);
    }

    int ThreadPool::getAvailable() {
        std::set<int> availableThreads;

        std::set_difference(
                this->allThreads.begin(), this->allThreads.end(),
                this->executingThreads.begin(), this->executingThreads.end(),
                std::inserter(availableThreads, availableThreads.end())
        );

        if(availableThreads.empty()) {
            throw NoThreadsException();
        }
        else {
            return *availableThreads.begin();
        }
    }

    void ThreadPool::setFinished(int threadIdx) {
        std::lock_guard<std::mutex> guard(threadsMutex);

        this->executingThreads.erase(threadIdx);
    }
}
