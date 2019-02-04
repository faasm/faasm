#pragma once

#include <util/config.h>
#include <util/func.h>
#include <util/queue.h>

namespace scheduler {
    class GlobalMessageBus {
    public:
        virtual void enqueueMessage(const message::Message &msg) = 0;

        virtual message::Message nextMessage() = 0;

        virtual message::Message nextMessage(int timeout) = 0;

        virtual void setFunctionResult(message::Message &msg, bool success) = 0;

        virtual message::Message getFunctionResult(const message::Message &msg) = 0;

    protected:
        GlobalMessageBus() : queueName(util::getSystemConfig().queueName) {
        }

        const std::string queueName;
    };

    class GlobalMessageBusNoMessageException : public std::exception {
    };
}