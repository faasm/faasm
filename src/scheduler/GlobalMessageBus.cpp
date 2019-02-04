#include "GlobalMessageBus.h"
#include "RedisMessageBus.h"
#include "AWSMessageBus.h"

#include <util/config.h>

namespace scheduler {
    GlobalMessageBus &GlobalMessageBus::getInstance() {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.systemMode == "redis") {
            static RedisMessageBus mb(INCOMING_QUEUE);
            return mb;
        } else if (conf.systemMode == "aws") {
            static AWSMessageBus mb(INCOMING_QUEUE);
            return mb;
        } else {
            throw std::runtime_error("Invalid system mode");
        }
    }

    GlobalMessageBus::GlobalMessageBus(const std::string &queueNameIn) : queueName(queueNameIn) {
        if (queueName.empty()) {
            throw std::runtime_error("Must provide a queue name");
        }
    }
}