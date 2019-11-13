#include "SharingMessageBus.h"

#include <util/logging.h>
#include <scheduler/Scheduler.h>

namespace scheduler {
    SharingMessageBus &SharingMessageBus::getInstance() {
        static thread_local SharingMessageBus mb;

        return mb;
    }

    std::string getSharingQueueNameForNode(const std::string &nodeId) {
        return SHARING_QUEUE_PREFIX + nodeId;
    }

    SharingMessageBus::SharingMessageBus() : conf(util::getSystemConfig()),
                                             thisNodeId(util::getNodeId()),
                                             redis(redis::Redis::getQueue()) {

    }

    message::Message SharingMessageBus::nextMessageForNode(const std::string &nodeId) {
        std::string queueName = getSharingQueueNameForNode(nodeId);
        std::vector<uint8_t> dequeueResult = redis.dequeueBytes(queueName, conf.globalMessageTimeout);

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        return msg;
    }

    message::Message SharingMessageBus::nextMessageForThisNode() {
        return this->nextMessageForNode(thisNodeId);
    }

    void SharingMessageBus::shareMessageWithNode(const std::string &nodeId, const message::Message &msg) {
        std::string queueName = getSharingQueueNameForNode(nodeId);
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
        redis.enqueueBytes(queueName, msgBytes);
    }

    void SharingMessageBus::broadcastMessage(const message::Message &msg) {
        std::unordered_set<std::string> allOptions = redis.smembers(GLOBAL_NODE_SET);

        for(auto &nodeId : allOptions) {
            shareMessageWithNode(nodeId, msg);
        }
    }
}