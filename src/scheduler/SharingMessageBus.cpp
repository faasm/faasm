#include "SharingMessageBus.h"

#include <util/logging.h>
#include <scheduler/Scheduler.h>

namespace scheduler {
    SharingMessageBus &SharingMessageBus::getInstance() {
        static thread_local SharingMessageBus mb;

        return mb;
    }

    std::string getSharingQueueNameForHost(const std::string &host) {
        return SHARING_QUEUE_PREFIX + host;
    }

    SharingMessageBus::SharingMessageBus() : conf(util::getSystemConfig()),
                                             thisHost(util::getSystemConfig().endpointHost),
                                             redis(redis::Redis::getQueue()) {

    }

    message::Message SharingMessageBus::nextMessageForHost(const std::string &host) {
        std::string queueName = getSharingQueueNameForHost(host);
        std::vector<uint8_t> dequeueResult = redis.dequeueBytes(queueName, conf.globalMessageTimeout);

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        return msg;
    }

    message::Message SharingMessageBus::nextMessageForThisHost() {
        return this->nextMessageForHost(thisHost);
    }

    void SharingMessageBus::shareMessageWithHost(const std::string &host, const message::Message &msg) {
        std::string queueName = getSharingQueueNameForHost(host);
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
        redis.enqueueBytes(queueName, msgBytes);
    }

    void SharingMessageBus::broadcastMessage(const message::Message &msg) {
        std::unordered_set<std::string> allOptions = redis.smembers(AVAILABLE_HOST_SET);

        for(auto &host : allOptions) {
            shareMessageWithHost(host, msg);
        }
    }
}