#include "aws.h"

#include <util/logging.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/sqs/model/GetQueueUrlRequest.h>
#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/sqs/model/ReceiveMessageRequest.h>
#include <aws/sqs/model/DeleteMessageRequest.h>

/**
 * Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html
 *
 * Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html
 */
namespace awswrapper {

    static Aws::SDKOptions options;

    Aws::Client::ClientConfiguration getClientConf(long timeout) {
        Aws::Client::ClientConfiguration conf;
        conf.region = Aws::Region::EU_WEST_1;
        conf.requestTimeoutMs = timeout;

        return conf;
    }

    void initSDK() {
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;

        Aws::InitAPI(options);
    }

    void cleanUpSDK() {
        Aws::ShutdownAPI(options);
    }

    template<typename T>
    void handleError(const Aws::Client::AWSError<T> err) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->error("Failed to list objects: {} {}", err.GetExceptionName().c_str(), err.GetMessage().c_str());

        throw std::runtime_error("AWS error");
    }


    // -------------------------
    // SQS
    // -------------------------

    SQSWrapper::SQSWrapper() : client(Aws::SQS::SQSClient(getClientConf(REQUEST_TIMEOUT_MS))) {
    }

    SQSWrapper &SQSWrapper::getThreadLocal() {
        static thread_local SQSWrapper sqs;
        return sqs;
    }

    std::string SQSWrapper::getQueueUrl(const std::string &queueName) {
        Aws::SQS::Model::GetQueueUrlRequest request;
        request.SetQueueName(Aws::String(queueName));

        auto response = client.GetQueueUrl(request);
        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }

        std::string url(response.GetResult().GetQueueUrl());
        return url;
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

    // -------------------------
    // S3
    // -------------------------

    S3Wrapper::S3Wrapper() : client(Aws::S3::S3Client(getClientConf(REQUEST_TIMEOUT_MS))) {

    }

    S3Wrapper &S3Wrapper::getThreadLocal() {
        static thread_local S3Wrapper s3;
        return s3;
    }


    std::vector<std::string> S3Wrapper::listKeys(const std::string &bucketName) {
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        Aws::S3::Model::ListObjectsRequest request;
        request.WithBucket(Aws::String(bucketName));

        auto response = s3.client.ListObjects(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }

        Aws::Vector<Aws::S3::Model::Object> keyObjects = response.GetResult().GetContents();
        std::vector<std::string> keys;
        for (auto const &keyObject : keyObjects) {
            const Aws::String &awsStr = keyObject.GetKey();
            keys.emplace_back(awsStr.c_str());
        }

        return keys;
    }

    void S3Wrapper::addKey(const std::string &bucketName, const std::string &keyName, const std::string &data) {
        Aws::S3::Model::PutObjectRequest request;
        request.WithBucket(Aws::String(bucketName)).WithKey(Aws::String(keyName));

        std::shared_ptr<Aws::IOStream> dataStream = Aws::MakeShared<Aws::StringStream>(data.c_str());
        *dataStream << data;
        request.SetBody(dataStream);

        auto response = client.PutObject(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }
    }

    std::string S3Wrapper::getKey(const std::string &bucketName, const std::string &keyName) {
        Aws::S3::Model::GetObjectRequest request;
        request.WithBucket(Aws::String(bucketName)).WithKey(Aws::String(keyName));

        auto response = client.GetObject(request);

        if (!response.IsSuccess()) {
            const auto &err = response.GetError();
            handleError(err);
        }

        std::ostringstream ss;
        auto responseStream = response.GetResult().GetBody().rdbuf();
        ss << responseStream;

        return ss.str();
    }
}
