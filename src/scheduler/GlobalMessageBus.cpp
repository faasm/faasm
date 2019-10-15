#include "Scheduler.h"
#include "GlobalMessageBus.h"
#include "RedisMessageBus.h"

#if AWS_SUPPORT
#include "AWSMessageBus.h"
#endif

#include <util/config.h>

namespace scheduler {
    GlobalMessageBus &getGlobalMessageBus() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.globalMessageBus == "redis") {
            static thread_local RedisMessageBus mb;
            return mb;
#if AWS_SUPPORT == 1
        } else if (conf.globalMessageBus == "sqs") {
            static thread_local AWSMessageBus mb;
            return mb;
#endif
        } else {
            throw std::runtime_error("Invalid global message bus type");
        }
    }
}