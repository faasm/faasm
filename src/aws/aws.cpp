#include "aws.h"

#include <util/util.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/sqs/model/SendMessageRequest.h>

/**
 * Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html
 *
 * Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html
 */
namespace awswrapper {

    static Aws::SDKOptions options;

    Aws::Client::ClientConfiguration getClientConf() {
        Aws::Client::ClientConfiguration conf;
        conf.region = Aws::Region::EU_WEST_1;

        return conf;
    }

    void initSDK() {
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;

        Aws::InitAPI(options);
    }

    void cleanUpSDK() {
        Aws::ShutdownAPI(options);
    }

    SQSWrapper::SQSWrapper() : client(Aws::SQS::SQSClient(getClientConf())) {

    }

    SQSWrapper &SQSWrapper::getThreadLocal() {
        static thread_local SQSWrapper sqs;
        return sqs;
    }

    S3Wrapper::S3Wrapper() : client(Aws::S3::S3Client(getClientConf())) {

    }

    S3Wrapper &S3Wrapper::getThreadLocal() {
        static thread_local S3Wrapper s3;
        return s3;
    }

    template<class T>
    void handleError(const Aws::Client::AWSError<T> err) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->error("Failed to list objects: {} {}", err.GetExceptionName().c_str(), err.GetMessage().c_str());

        throw std::runtime_error("AWS error");
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
