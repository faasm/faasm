#include "Scheduler.h"
#include "GlobalMessageBus.h"
#include "RedisMessageBus.h"
#include "AWSMessageBus.h"

#include <util/config.h>

namespace scheduler {
    GlobalMessageBus &getGlobalMessageBus() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.systemMode == "redis") {
            static RedisMessageBus mb;
            return mb;
        } else if (conf.systemMode == "aws") {
            static AWSMessageBus mb;
            return mb;
        } else {
            throw std::runtime_error("Invalid system mode");
        }
    }
}