#pragma once

#include "GlobalMessageBus.h"

#include <aws/SQSWrapper.h>
#include <aws/S3Wrapper.h>

#include <util/config.h>
#include <aws/LambdaWrapper.h>

namespace scheduler {
    class AWSMessageBus : public GlobalMessageBus {
    public:
        AWSMessageBus();

        void enqueueMessage(const message::Message &msg) override;

        message::Message nextMessage() override;

        message::Message nextMessage(int timeout) override;

        std::string getMessageStatus(unsigned int messageId) override;

        void setFunctionResult(message::Message &msg, bool success) override;

        message::Message getFunctionResult(unsigned int messageId, int timeout) override;

        void clear() override;

    private:
        awswrapper::SQSWrapper &sqs;
        awswrapper::S3Wrapper &s3;
        awswrapper::LambdaWrapper &lambda;
        const std::string bucketName;
    };
}