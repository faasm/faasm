#include "Scheduler.h"
#include "GlobalMessageBus.h"
#include "RedisMessageBus.h"

#include <util/config.h>

namespace scheduler {
    GlobalMessageBus &getGlobalMessageBus() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.globalMessageBus == "redis") {
            static thread_local RedisMessageBus mb;
            return mb;
        } else {
            throw std::runtime_error("Invalid global message bus type");
        }
    }
}