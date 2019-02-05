#pragma once

#include "aws.h"

#include <aws/s3/S3Client.h>

namespace awswrapper {
    class S3Wrapper {
    public:
        static const long REQUEST_TIMEOUT_MS = 500;

        static S3Wrapper &getThreadLocal();

        S3Wrapper();

        std::vector<std::string> listKeys(const std::string &bucketName);

        void addKeyBytes(const std::string &bucketName, const std::string &keyName, const std::vector<uint8_t> &data);

        void addKeyStr(const std::string &bucketName, const std::string &keyName, const std::string &data);

        std::vector<uint8_t> getKeyBytes(const std::string &bucketName, const std::string &keyName);

        std::string getKeyStr(const std::string &bucketName, const std::string &keyName);

    private:
        void addKey(const std::string &bucketName, const std::string &keyName, const char *data, size_t dataLen);

        std::string getKey(const std::string &bucketName, const std::string &keyName);

        Aws::S3::S3Client client;
    };
}