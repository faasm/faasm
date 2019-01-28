#pragma once

#include <aws/core/Aws.h>

namespace awswrapper {

    void initSDK();
    void cleanUpSDK();

    class SQSWrapper {
    public:
        void connect();
    };

    class S3Wrapper {
    public:
        void listKeys(std::string bucketName);
    };
}