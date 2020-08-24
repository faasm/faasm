#include "barrier.h"
#include "locks.h"

namespace faabric::util {
    Barrier::Barrier(int count): threadCount(count), slotCount(count), uses(0) {

    }

    void Barrier::wait() {
        {
            UniqueLock lock(mx);
            int usesCopy = uses;

            slotCount--;
            if(slotCount == 0) {
                uses++;
                // Checks for overflow
                if (uses < 0) {
                    throw std::runtime_error("Barrier was used too many times");
                }
                slotCount = threadCount;
                cv.notify_all();
            } else {
                cv.wait(lock, [&]{return usesCopy < uses;});
            }
        }

    }

    int Barrier::getSlotCount() {
        return slotCount;
    }

    int Barrier::getUseCount() {
        return uses;
    }

}