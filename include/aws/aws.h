#pragma once

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/sqs/SQSClient.h>

namespace awswrapper {

    Aws::Client::ClientConfiguration getClientConf();

    void initSDK();
    void cleanUpSDK();

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

    class S3Wrapper {
    public:
        static const long REQUEST_TIMEOUT_MS = 500;

        static S3Wrapper& getThreadLocal();

        S3Wrapper();

        std::vector<std::string> listKeys(const std::string &bucketName);

        void addKey(const std::string &bucketName, const std::string &keyName, const std::string &data);

        std::string getKey(const std::string &bucketName, const std::string &keyName);
    private:
        Aws::S3::S3Client client;
    };
}