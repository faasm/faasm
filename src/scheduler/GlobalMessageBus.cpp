#include "Scheduler.h"
#include "GlobalMessageBus.h"
#include "RedisMessageBus.h"
#include "AWSMessageBus.h"

#include <util/config.h>

namespace scheduler {
    GlobalMessageBus &getGlobalMessageBus() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.globalMessageBus == "redis") {
            static thread_local RedisMessageBus mb;
            return mb;
        } else if (conf.globalMessageBus == "sqs") {
            static thread_local AWSMessageBus mb;
            return mb;
        } else {
            throw std::runtime_error("Invalid global message bus type");
        }
    }
}