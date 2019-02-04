#pragma once

#include <util/func.h>
#include <util/queue.h>
#include <redis/Redis.h>

#define INCOMING_QUEUE "all_incoming"

namespace scheduler {
    class GlobalMessageBus {
    public:
        explicit GlobalMessageBus(const std::string &queueName);

        const std::string queueName;

        virtual void enqueueMessage(const message::Message &msg);

        virtual message::Message nextMessage(int timeout);

        virtual void setFunctionResult(message::Message &msg, bool success);

        virtual message::Message getFunctionResult(const message::Message &msg);
    private:
        redis::Redis &redis;
    };
}