#pragma once

#include "GlobalMessageBus.h"

#include <redis/Redis.h>

namespace scheduler {
    class RedisMessageBus : public GlobalMessageBus {
    public:
        explicit RedisMessageBus(const std::string &queueName);

        void enqueueMessage(const message::Message &msg) override;

        message::Message nextMessage() override;

        message::Message nextMessage(int timeout) override;

        void setFunctionResult(message::Message &msg, bool success) override;

        message::Message getFunctionResult(const message::Message &msg) override;

    private:
        redis::Redis &redis;
    };
}