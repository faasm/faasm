#include "SharingMessageBus.h"

#include <util/logging.h>

namespace scheduler {
    SharingMessageBus &SharingMessageBus::getInstance() {
        static SharingMessageBus mb;

        return mb;
    }

    std::string getSharingQueueNameForHost(const std::string &hostname) {
        return "sharing_" + hostname;
    }

    SharingMessageBus::SharingMessageBus() : hostname(util::getHostName()), redis(redis::Redis::getQueue()) {

    }

    message::Message SharingMessageBus::nextMessageForThisHost() {
        std::string queueName = getSharingQueueNameForHost(hostname);
        std::vector<uint8_t> dequeueResult = redis.dequeue(queueName, util::DEFAULT_TIMEOUT);

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        return msg;
    }

    void SharingMessageBus::shareMessageWithHost(const std::string &hostname, const message::Message &msg) {
        std::string queueName = getSharingQueueNameForHost(hostname);
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
        redis.enqueue(queueName, msgBytes);
    }
}