#pragma once

#include <util/func.h>
#include <util/queue.h>

#define INCOMING_QUEUE "all_incoming"

namespace scheduler {
    class GlobalMessageBus {
    public:
        const std::string queueName;

        static GlobalMessageBus &getInstance();

        virtual void enqueueMessage(const message::Message &msg);

        virtual message::Message nextMessage();

        virtual message::Message nextMessage(int timeout);

        virtual void setFunctionResult(message::Message &msg, bool success);

        virtual message::Message getFunctionResult(const message::Message &msg);

    protected:
        explicit GlobalMessageBus(const std::string &queueName);
    };
}