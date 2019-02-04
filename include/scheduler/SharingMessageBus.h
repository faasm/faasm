#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>


namespace scheduler {
    class SharingMessageBus {
    public:
        SharingMessageBus();

        static SharingMessageBus &getInstance();

        message::Message nextMessageForThisHost();

        void shareMessageWithHost(const std::string &hostname, const message::Message &msg);
    private:
        std::string hostname;
        redis::Redis &redis;
    };
}