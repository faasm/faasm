#pragma once

#include <memory>
#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/S3Client.h>

namespace storage {

std::shared_ptr<Aws::Auth::AWSCredentialsProvider> getCredentialsProvider();

Aws::Client::ClientConfiguration getClientConf(long timeout = 3000);

void initSDK();
void cleanUpSDK();

template<typename T>
void handleError(const Aws::Client::AWSError<T> err)
{
    SPDLOG_ERROR("AWS error: {} {}",
                 err.GetExceptionName().c_str(),
                 err.GetMessage().c_str());

    throw std::runtime_error("AWS error");
}

class S3Wrapper
{
  public:
    static const long REQUEST_TIMEOUT_MS = 500;

    static S3Wrapper& getThreadLocal();

    S3Wrapper();

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
    void addKey(const std::string& bucketName,
                const std::string& keyName,
                const char* data,
                size_t dataLen);

    std::string getKey(const std::string& bucketName,
                       const std::string& keyName);

    std::shared_ptr<Aws::Auth::AWSCredentialsProvider> credentialsProvider;
    Aws::Client::ClientConfiguration clientConf;
    Aws::S3::S3Client client;
};
}
