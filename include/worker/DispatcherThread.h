#pragma once

#include <scheduler/Scheduler.h>
#include <util/func.h>

#include <string>


namespace worker {
    class DispatcherThread {
    public:
        explicit DispatcherThread(const std::string &queueName);

        void run();
    private:
        std::string queueName;
    };
}