#pragma once

#include "GlobalMessageBus.h"

#include <aws/SQSWrapper.h>
#include <aws/S3Wrapper.h>

#include <util/config.h>

namespace scheduler {
    class AWSMessageBus : public GlobalMessageBus {
    public:
        AWSMessageBus();

        void enqueueMessage(const message::Message &msg);

        message::Message nextMessage();

        message::Message nextMessage(int timeout);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);

    private:
        awswrapper::SQSWrapper &sqs;
        awswrapper::S3Wrapper &s3;
        const std::string bucketName;
    };
}