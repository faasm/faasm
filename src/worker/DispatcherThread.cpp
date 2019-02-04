#include "DispatcherThread.h"

#include <util/logging.h>

namespace worker {
    static const long timeout = 5 * 60 * 1000;

    DispatcherThread::DispatcherThread(const std::string &queueNameIn) :
            queueName(queueNameIn) {

    }

    void DispatcherThread::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        scheduler::Scheduler &sch = scheduler::getScheduler();
        scheduler::GlobalMessageBus queue(queueName);

        // Wait on next message
        while (true) {
            try {
                message::Message msg = queue.nextMessage(timeout);
                sch.callFunction(msg);
            }
            catch (redis::RedisNoResponseException ex) {
                logger->info("No message from global bus in timeout");
                continue;
            }
        }

    }
}