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
        static SQSWrapper& getThreadLocal();

        SQSWrapper();

    private:
        Aws::SQS::SQSClient client;
    };

    class S3Wrapper {
    public:
        static S3Wrapper& getThreadLocal();

        S3Wrapper();

        void listKeys(std::string bucketName);
    private:
        Aws::S3::S3Client client;
    };
}