#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>


namespace scheduler {
    class SharingMessageBus {
    public:
        SharingMessageBus();

        static SharingMessageBus &getInstance();

        message::Message nextMessageForHost(const std::string &hostname);

        message::Message nextMessageForThisHost();

        void shareMessageWithHost(const std::string &hostname, const message::Message &msg);
    private:
        util::SystemConfig &conf;
        std::string thisHostname;
        redis::Redis &redis;
    };
}