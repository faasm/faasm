#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>

#define INCOMING_QUEUE "all_incoming"

namespace scheduler {
    class GlobalMessageQueue {
    public:
        explicit GlobalMessageQueue(const std::string &queueName);

        const std::string queueName;

        void enqueueMessage(const message::Message &msg);

        message::Message nextMessage(int timeout = util::DEFAULT_TIMEOUT);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);
    private:
        redis::Redis &redis;
    };
}