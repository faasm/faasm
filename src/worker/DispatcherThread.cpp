#include "DispatcherThread.h"

namespace worker {
    static const long timeout = 5 * 60 * 1000;

    DispatcherThread::DispatcherThread(scheduler::GlobalMessageQueue &queue) :
        queue(queue),
        sch(scheduler::getScheduler()) {

    }

    void DispatcherThread::run() {
        // Wait on next message
        while(true) {
            message::Message msg = queue.nextMessage(timeout);
            sch.callFunction(msg);
        }

    }
}