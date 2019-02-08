#include "aws.h"
#include "SQSWrapper.h"

#include <util/logging.h>
#include <util/locks.h>

#include <aws/sqs/SQSClient.h>

#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/sqs/model/GetQueueUrlRequest.h>
#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/sqs/model/ReceiveMessageRequest.h>
#include <aws/sqs/model/DeleteMessageRequest.h>

namespace awswrapper {
    std::unordered_map<std::string, std::string> urlMap;
    std::mutex urlMapMx;

    SQSWrapper::SQSWrapper() : client(Aws::SQS::SQSClient(getClientConf(REQUEST_TIMEOUT_MS))) {
    }

    SQSWrapper &SQSWrapper::getThreadLocal() {
        static thread_local SQSWrapper sqs;
        return sqs;
    }

    std::string SQSWrapper::getQueueUrl(const std::string &queueName) {
        if(urlMap.count(queueName) == 0) {
            // Make sure we have a full lock before getting the URL for this queue
            util::UniqueLock lock(urlMapMx);

            // Double check condition
            if(urlMap.count(queueName) == 0) {
                Aws::SQS::Model::GetQueueUrlRequest request;
                request.SetQueueName(Aws::String(queueName));

                auto response = client.GetQueueUrl(request);
                if (!response.IsSuccess()) {
                    const auto &err = response.GetError();
                    handleError(err);
                }

                std::string url(response.GetResult().GetQueueUrl());
                urlMap[queueName] = url;
            }
        }

        return urlMap[queueName];
    }

    void SQSWrapper::sendMessage(const std::string &queueUrl, const std::string &content) {
        Aws::SQS::Model::SendMessageRequest request;
        request.SetQueueUrl(Aws::String(queueUrl));
        request.SetMessageBody(Aws::String(content));

        auto response = client.SendMessage(request);
        if (!response.IsSuccess()) {
            handleError(response.GetError());
        }
    }

    /**
     * Note: SQS messages have a "visibility timeout" which means, when one
     * reader gets a message, it won't be visible to others within this timeout,
     * thus giving the reader time enough to delete the message without locking
     * (if it wants to do so).
     */
    std::string SQSWrapper::receiveMessage(const std::string &queueUrl) {
        Aws::SQS::Model::ReceiveMessageRequest request;
        request.SetQueueUrl(Aws::String(queueUrl));
        request.SetMaxNumberOfMessages(1);

        auto response = client.ReceiveMessage(request);
        if (!response.IsSuccess())  {
            handleError(response.GetError());
        }

        const auto& messages = response.GetResult().GetMessages();

        // No messages
        if (messages.empty()) {
            std::string empty;
            return empty;
        }

        const auto& message = messages[0];
        std::string msgBody(message.GetBody());
        
        // snippet-start:[sqs.cpp.delete_message.code]
        Aws::SQS::Model::DeleteMessageRequest deleteRequest;
        deleteRequest.SetQueueUrl(Aws::String(queueUrl));
        deleteRequest.SetReceiptHandle(message.GetReceiptHandle());

        auto deleteResponse = client.DeleteMessage(deleteRequest);
        if (!deleteResponse.IsSuccess())        {
            handleError(deleteResponse.GetError());
        }

        return msgBody;
    }
}
