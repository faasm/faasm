#include "AWSMessageBus.h"

#include <util/logging.h>

namespace scheduler {
    // TODO change from thread-local to singleton SQS and S3 connections (make thread safe?)
    AWSMessageBus::AWSMessageBus(const std::string &queueNameIn) : GlobalMessageBus(queueNameIn),
                                                                   sqs(awswrapper::SQSWrapper::getThreadLocal()),
                                                                   s3(awswrapper::S3Wrapper::getThreadLocal()) {
        
        
    }

    void AWSMessageBus::enqueueMessage(const message::Message &msg) {
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);

        // TODO serialise to string and send
        // TODO cache queue URL

        const std::string url = sqs.getQueueUrl(queueName);
        sqs.sendMessage(url, msgBytes);
    }

    message::Message AWSMessageBus::nextMessage() {
        const std::string url = sqs.getQueueUrl(queueName);
        sqs.receiveMessage(url);

        // TODO deserialise into message
    }

    message::Message AWSMessageBus::nextMessage(int timeout) {
        // TODO dequeue from SQS
    }

    void AWSMessageBus::setFunctionResult(message::Message &msg, bool success) {
        // TODO set in S3
    }

    message::Message AWSMessageBus::getFunctionResult(const message::Message &msg) {
       // TODO get from S3
    }
}