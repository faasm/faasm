#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>

#define SHARING_QUEUE_PREFIX "sharing_"

namespace scheduler {
    std::string getSharingQueueNameForNode(const std::string &nodeId);

    class SharingMessageBus {
    public:
        SharingMessageBus();

        static SharingMessageBus &getInstance();

        message::Message nextMessageForNode(const std::string &nodeId);

        message::Message nextMessageForThisNode();

        void shareMessageWithNode(const std::string &nodeId, const message::Message &msg);

        void broadcastMessage(const message::Message &msg);
    private:
        util::SystemConfig &conf;
        std::string thisNodeId;
        redis::Redis &redis;
    };
}