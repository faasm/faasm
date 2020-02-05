#include "util/barrier.h"
#include "util/locks.h"

namespace util {
    Barrier::Barrier(int count): threadCount(count), slotCount(count), uses(0) {

    }

    void Barrier::wait() {
        {
            UniqueLock lock(mx);
            int usesCopy = uses;

            slotCount--;
            if(slotCount == 0) {
                uses++;
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