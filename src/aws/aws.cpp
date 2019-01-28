#include "aws.h"

#include <aws/sqs/model/SendMessageRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>

// Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html

// Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html

namespace awswrapper {

    static Aws::SDKOptions options;

    Aws::Client::ClientConfiguration getClientConf() {
        Aws::Client::ClientConfiguration conf;
        conf.region = "eu-west-1";

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

    void S3Wrapper::listKeys(std::string bucketName) {
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        Aws::S3::Model::ListObjectsRequest objects_request;
        objects_request.WithBucket(Aws::String(bucketName));

        auto list_objects_outcome = s3.client.ListObjects(objects_request);

        if (list_objects_outcome.IsSuccess()) {
            Aws::Vector<Aws::S3::Model::Object> object_list =
                    list_objects_outcome.GetResult().GetContents();

            for (auto const &s3_object : object_list) {
                std::cout << "* " << s3_object.GetKey() << std::endl;
            }
        } else {
            std::cout << "ListObjects error: " <<
                      list_objects_outcome.GetError().GetExceptionName() << " " <<
                      list_objects_outcome.GetError().GetMessage() << std::endl;
        }
    }
}
