#pragma once

#include <scheduler/Scheduler.h>
#include <util/func.h>

#include <string>


namespace worker {
    class DispatcherThread {
    public:
        explicit DispatcherThread(scheduler::GlobalMessageQueue &queue);

        void run();
    private:
        scheduler::GlobalMessageQueue queue;
        scheduler::Scheduler &sch;
    };
}