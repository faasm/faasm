#include "SharingMessageBus.h"

#include <util/logging.h>

namespace scheduler {
    SharingMessageBus &SharingMessageBus::getInstance() {
        static thread_local SharingMessageBus mb;

        return mb;
    }

    std::string getSharingQueueNameForNode(const std::string &nodeId) {
        return "sharing_" + nodeId;
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
}