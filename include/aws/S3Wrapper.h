#pragma once

#include "aws.h"
#include <aws/s3/S3Client.h>

namespace awswrapper {
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