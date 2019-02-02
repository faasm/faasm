#pragma once

#include "aws.h"
#include <aws/sqs/SQSClient.h>

namespace awswrapper {
    class SQSWrapper {
    public:
        static const long REQUEST_TIMEOUT_MS = 30000;

        static SQSWrapper& getThreadLocal();

        SQSWrapper();

        std::string getQueueUrl(const std::string &queueName);

        void sendMessage(const std::string &queueUrl, const std::string &content);

        std::string receiveMessage(const std::string &queueUrl);

    private:
        Aws::SQS::SQSClient client;
    };
}