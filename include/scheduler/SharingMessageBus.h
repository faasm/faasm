#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>

#define SHARING_QUEUE_PREFIX "sharing_"

namespace scheduler {
    std::string getSharingQueueNameForHost(const std::string &host);

    class SharingMessageBus {
    public:
        SharingMessageBus();

        static SharingMessageBus &getInstance();

        message::Message nextMessageForHost(const std::string &host);

        message::Message nextMessageForThisHost();

        void shareMessageWithHost(const std::string &host, const message::Message &msg);

        void broadcastMessage(const message::Message &msg);
    private:
        util::SystemConfig &conf;
        std::string thisHost;
        redis::Redis &redis;
    };
}