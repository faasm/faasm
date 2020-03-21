#pragma once

#include "GlobalMessageBus.h"

#include <redis/Redis.h>

namespace scheduler {
    class RedisMessageBus : public GlobalMessageBus {
    public:
        RedisMessageBus();

        void enqueueMessage(const message::Message &msg) override;

        message::Message nextMessage() override;

        message::Message nextMessage(int timeout) override;

        std::string getMessageStatus(unsigned int messageId) override;

        void setFunctionResult(message::Message &msg) override;

        message::Message getFunctionResult(unsigned int messageId, int timeoutMs) override;

        void clear() override;

    private:
        redis::Redis &redis;
    };
}