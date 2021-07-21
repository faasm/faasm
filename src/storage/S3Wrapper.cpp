#include <conf/FaasmConfig.h>
#include <storage/S3Wrapper.h>

#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>

#include <aws/s3/S3Errors.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

using namespace Aws::S3::Model;
using namespace Aws::Client;
using namespace Aws::Auth;

namespace storage {

static Aws::SDKOptions options;

template<typename R>
R reqFactory(const std::string& bucket)
{
    R req;
    req.SetBucket(bucket);
    return req;
}

template<typename R>
R reqFactory(const std::string& bucket, const std::string& key)
{
    R req = reqFactory<R>(bucket);
    req.SetKey(key);
    return req;
}

#define CHECK_ERRORS(response)                                                 \
    {                                                                          \
        if (!response.IsSuccess()) {                                           \
            const auto& err = response.GetError();                             \
            SPDLOG_ERROR("S3 error: {} {}",                                    \
                         err.GetExceptionName().c_str(),                       \
                         err.GetMessage().c_str());                            \
            throw std::runtime_error("S3 error");                              \
        }                                                                      \
    }

std::shared_ptr<AWSCredentialsProvider> getCredentialsProvider()
{
    return Aws::MakeShared<ProfileConfigFileAWSCredentialsProvider>("local");
}

ClientConfiguration getClientConf(long timeout)
{
    // There are a couple of conflicting pieces of info on how to configure
    // the AWS C++ SDK for use with minio:
    // https://stackoverflow.com/questions/47105289/how-to-override-endpoint-in-aws-sdk-cpp-to-connect-to-minio-server-at-localhost
    // https://github.com/aws/aws-sdk-cpp/issues/587
    ClientConfiguration config;

    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    config.region = "";
    config.verifySSL = false;
    config.endpointOverride = faasmConf.s3Host + ":" + faasmConf.s3Port;
    config.connectTimeoutMs = S3_CONNECT_TIMEOUT_MS;
    config.requestTimeoutMs = timeout;

    // Use HTTP, not HTTPS
    config.scheme = Aws::Http::Scheme::HTTP;

    return config;
}

void initSDK()
{
    SPDLOG_INFO("Initialising AWS SDK");
    Aws::InitAPI(options);
}

void cleanUpSDK()
{
    Aws::ShutdownAPI(options);
}

S3Wrapper::S3Wrapper()
  : faasmConf(conf::getFaasmConfig())
  , clientConf(getClientConf(S3_REQUEST_TIMEOUT_MS))
  , client(AWSCredentials(faasmConf.s3User, faasmConf.s3Password),
           clientConf,
           AWSAuthV4Signer::PayloadSigningPolicy::Never,
           false)
{}

void S3Wrapper::createBucket(const std::string& bucketName)
{
    SPDLOG_DEBUG("Creating bucket {}", bucketName);
    auto request = reqFactory<CreateBucketRequest>(bucketName);
    auto response = client.CreateBucket(request);

    if (!response.IsSuccess()) {
        const auto& err = response.GetError();

        auto errType = err.GetErrorType();
        if (errType == Aws::S3::S3Errors::BUCKET_ALREADY_OWNED_BY_YOU ||
            errType == Aws::S3::S3Errors::BUCKET_ALREADY_EXISTS) {
            SPDLOG_DEBUG("Bucket already exists {}", bucketName);
        } else {
            CHECK_ERRORS(response);
        }
    }
}

std::vector<std::string> S3Wrapper::listBuckets()
{
    SPDLOG_DEBUG("Listing buckets");
    auto response = client.ListBuckets();
    CHECK_ERRORS(response);

    Aws::Vector<Bucket> bucketObjects = response.GetResult().GetBuckets();

    std::vector<std::string> bucketNames;
    for (auto const& bucketObject : bucketObjects) {
        const Aws::String& awsStr = bucketObject.GetName();
        bucketNames.emplace_back(awsStr.c_str());
    }

    return bucketNames;
}

std::vector<std::string> S3Wrapper::listKeys(const std::string& bucketName)
{
    SPDLOG_DEBUG("Listing keys in bucket {}", bucketName);
    auto request = reqFactory<ListObjectsRequest>(bucketName);
    auto response = client.ListObjects(request);

    CHECK_ERRORS(response);

    Aws::Vector<Object> keyObjects = response.GetResult().GetContents();

    std::vector<std::string> keys;
    for (auto const& keyObject : keyObjects) {
        const Aws::String& awsStr = keyObject.GetKey();
        keys.emplace_back(awsStr.c_str());
    }

    return keys;
}

void S3Wrapper::deleteKey(const std::string& bucketName,
                          const std::string& keyName)
{
    SPDLOG_DEBUG("Deleting key {}/{}", bucketName, keyName);
    auto request = reqFactory<DeleteObjectRequest>(bucketName, keyName);
    auto response = client.DeleteObject(request);
    CHECK_ERRORS(response);
}

void S3Wrapper::addKeyBytes(const std::string& bucketName,
                            const std::string& keyName,
                            const std::vector<uint8_t>& data)
{
    // See example:
    // https://github.com/awsdocs/aws-doc-sdk-examples/blob/master/cpp/example_code/s3/put_object_buffer.cpp
    SPDLOG_DEBUG("Writing {} to bucket {} as bytes", keyName, bucketName);
    auto request = reqFactory<PutObjectRequest>(bucketName, keyName);

    const std::shared_ptr<Aws::IOStream> dataStream =
      Aws::MakeShared<Aws::StringStream>((char*)data.data());
    dataStream->write((char*)data.data(), data.size());
    dataStream->flush();

    request.SetBody(dataStream);

    auto response = client.PutObject(request);
    CHECK_ERRORS(response);
}

void S3Wrapper::addKeyStr(const std::string& bucketName,
                          const std::string& keyName,
                          const std::string& data)
{
    // See example:
    // https://github.com/awsdocs/aws-doc-sdk-examples/blob/master/cpp/example_code/s3/put_object_buffer.cpp
    SPDLOG_DEBUG("Writing {} to bucket {} as string", keyName, bucketName);

    auto request = reqFactory<PutObjectRequest>(bucketName, keyName);

    const std::shared_ptr<Aws::IOStream> dataStream =
      Aws::MakeShared<Aws::StringStream>("");
    *dataStream << data;
    dataStream->flush();

    request.SetBody(dataStream);
    auto response = client.PutObject(request);
    CHECK_ERRORS(response);
}

std::vector<uint8_t> S3Wrapper::getKeyBytes(const std::string& bucketName,
                                            const std::string& keyName)
{
    SPDLOG_DEBUG("Getting key {}/{} as bytes", bucketName, keyName);
    auto request = reqFactory<GetObjectRequest>(bucketName, keyName);
    GetObjectOutcome response = client.GetObject(request);
    CHECK_ERRORS(response);

    std::vector<uint8_t> rawData(response.GetResult().GetContentLength());
    response.GetResult().GetBody().read((char*)rawData.data(), rawData.size());
    return rawData;
}

std::string S3Wrapper::getKeyStr(const std::string& bucketName,
                                 const std::string& keyName)
{
    SPDLOG_DEBUG("Getting key {}/{} as string", bucketName, keyName);
    auto request = reqFactory<GetObjectRequest>(bucketName, keyName);
    GetObjectOutcome response = client.GetObject(request);
    CHECK_ERRORS(response);

    std::ostringstream ss;
    auto* responseStream = response.GetResultWithOwnership().GetBody().rdbuf();
    ss << responseStream;

    return ss.str();
}
}
