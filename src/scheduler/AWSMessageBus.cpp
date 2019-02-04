#include "AWSMessageBus.h"

#include <util/logging.h>
#include <util/json.h>
#include <util/base64.h>


namespace scheduler {
    AWSMessageBus::AWSMessageBus() : sqs(awswrapper::SQSWrapper::getThreadLocal()),
                                     s3(awswrapper::S3Wrapper::getThreadLocal()),
                                     bucketName(util::getSystemConfig().bucketName) {

    }

    void AWSMessageBus::enqueueMessage(const message::Message &msg) {
        std::string msgBytes = msg.SerializeAsString();

        const std::string url = sqs.getQueueUrl(conf.queueName);

        if (conf.serialisation == "proto") {
            // Annoyingly SQS has restrictions on which characters it allows in messages
            // The consensus is that encoding/ decoding with base64 is acceptable 
            // Unfortunately there is no obvious/ easy C++ base64 library so the current
            // solution is copy-pasted. As a result performance may/ may not become an issue.
            const std::string encoded = util::b64encode(msgBytes);
            sqs.sendMessage(url, encoded);
        } else {
            const std::string &json = util::messageToJson(msg);
            sqs.sendMessage(url, json);
        }
    }

    message::Message AWSMessageBus::nextMessage() {
        const std::string url = sqs.getQueueUrl(conf.queueName);

        // TODO throw appropriate exception when timing out or no messages found
        const std::string result = sqs.receiveMessage(url);

        if (conf.serialisation == "proto") {
            const std::string decoded = util::b64decode(result);
            message::Message msg;
            msg.ParseFromString(decoded);

            return msg;
        } else {
            message::Message msg = util::jsonToMessage(result);
            return msg;
        }
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