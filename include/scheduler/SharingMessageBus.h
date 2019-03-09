#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>


namespace scheduler {
    class SharingMessageBus {
    public:
        SharingMessageBus();

        static SharingMessageBus &getInstance();

        message::Message nextMessageForNode(const std::string &nodeId);

        message::Message nextMessageForThisNode();

        void shareMessageWithNode(const std::string &nodeId, const message::Message &msg);
    private:
        util::SystemConfig &conf;
        std::string thisNodeId;
        redis::Redis &redis;
    };
}