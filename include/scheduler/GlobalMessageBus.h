#pragma once

#include <util/config.h>
#include <util/exception.h>
#include <util/func.h>
#include <util/queue.h>

namespace scheduler {
    class GlobalMessageBus {
    public:
        virtual void enqueueMessage(const message::Message &msg) = 0;

        virtual message::Message nextMessage() = 0;

        virtual message::Message nextMessage(int timeout) = 0;

        virtual std::string getMessageStatus(unsigned int messageId) = 0;

        virtual void setFunctionResult(message::Message &msg) = 0;

        virtual message::Message getFunctionResult(unsigned int messageId, int timeout) = 0;

        virtual void logChainedFunction(unsigned int parentMessageId, unsigned int chainedMessageId) = 0;

        virtual std::vector<unsigned int> getChainedFunctions(unsigned int msgId) = 0;

        virtual void clear() = 0;
    protected:
        GlobalMessageBus() : conf(util::getSystemConfig()){
        }

        const util::SystemConfig &conf;
    };

    class GlobalMessageBusNoMessageException : public util::FaasmException {
    public:
        explicit GlobalMessageBusNoMessageException(std::string message): FaasmException(std::move(message)) {

        }
    };
}