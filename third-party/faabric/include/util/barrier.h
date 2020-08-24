#pragma once

#include <mutex>
#include <condition_variable>

namespace util {

    class Barrier {
    public:
        explicit Barrier(int count);

        void wait();

        int getSlotCount();

        int getUseCount();
    private:
        int threadCount;
        int slotCount;
        int uses;
        std::mutex mx;
        std::condition_variable cv;
    };
}