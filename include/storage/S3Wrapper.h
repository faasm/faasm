#pragma once

#include <conf/FaasmConfig.h>
#include <faabric/util/logging.h>
#include <miniocpp/client.h>

#include <string>
#include <vector>

#define S3_REQUEST_TIMEOUT_MS 10000
#define S3_CONNECT_TIMEOUT_MS 500

namespace storage {

void initFaasmS3();

void shutdownFaasmS3();

class S3Wrapper
{
  public:
    S3Wrapper();

    void createBucket(const std::string& bucketName);

    void deleteBucket(const std::string& bucketName, bool recursive = false);

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
                                     const std::string& keyName,
                                     bool tolerateMissing = false);

    std::string getKeyStr(const std::string& bucketName,
                          const std::string& keyName,
                          bool tolerateMissing = false);

  private:
    const conf::FaasmConfig& faasmConf;
    minio::s3::BaseUrl baseUrl;
    minio::creds::StaticProvider provider;
    minio::s3::Client client;
};
}
