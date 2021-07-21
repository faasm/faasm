#pragma once

#include <memory>
#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>

#include <faabric/util/logging.h>

namespace storage {

Aws::Client::ClientConfiguration getClientConf(long timeout = 3000);

#define S3_REQUEST_TIMEOUT_MS 500

#define MINIO_CONNECT_TIMEOUT_MS 500
#define MINIO_URI "minio:9000"
#define MINIO_USER "minio"
#define MINIO_PASSWORD "minio123"

void initSDK();

void cleanUpSDK();

class S3Wrapper
{
  public:
    S3Wrapper();

    void createBucket(const std::string& bucketName);

    std::vector<std::string> listBuckets();

    std::vector<std::string> listKeys(const std::string& bucketName);

    void deleteKey(const std::string& bucketName, const std::string& keyName);

    void addKeyBytes(const std::string& bucketName,
                     const std::string& keyName,
                     const std::vector<uint8_t>& data);

    void addKeyStr(const std::string& bucketName,
                   const std::string& keyName,
                   const std::string& data);

    std::vector<uint8_t> getKeyBytes(const std::string& bucketName,
                                     const std::string& keyName);

    std::string getKeyStr(const std::string& bucketName,
                          const std::string& keyName);

  private:
    Aws::Client::ClientConfiguration clientConf;
    Aws::S3::S3Client client;
};
}
