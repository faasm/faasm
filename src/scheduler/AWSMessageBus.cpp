#include "AWSMessageBus.h"

#include <util/logging.h>


namespace scheduler {
    AWSMessageBus::AWSMessageBus() : sqs(awswrapper::SQSWrapper::getThreadLocal()),
                                     s3(awswrapper::S3Wrapper::getThreadLocal()),
                                     bucketName(util::getSystemConfig().bucketName) {

    }

    void AWSMessageBus::enqueueMessage(const message::Message &msg) {
        std::string msgBytes = msg.SerializeAsString();
        const std::string url = sqs.getQueueUrl(queueName);

        sqs.sendMessage(url, msgBytes);
    }

    message::Message AWSMessageBus::nextMessage() {
        const std::string url = sqs.getQueueUrl(queueName);

        // TODO throw appropriate exception when timing out or no messages found
        const std::string result = sqs.receiveMessage(url);

        message::Message msg;
        msg.ParseFromString(result);

        return msg;
    }

    message::Message AWSMessageBus::nextMessage(int timeout) {
        // TODO respect timeouts
        return this->nextMessage();
    }

    void AWSMessageBus::setFunctionResult(message::Message &msg, bool success) {
        // TODO work out what format to write results to S3 in
        std::string s3Key = msg.resultkey();

    }

    message::Message AWSMessageBus::getFunctionResult(const message::Message &msg) {
        // TODO work out what format results from S3 are in
        std::string s3Key = msg.resultkey();

        const std::string result = s3.getKey(bucketName, s3Key);

        message::Message resultMsg;
        resultMsg.ParseFromString(result);
        return resultMsg;
    }
}