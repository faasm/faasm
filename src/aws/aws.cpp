#include "aws.h"

#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/Object.h>

// Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html

// Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html

namespace awswrapper {

    static Aws::SDKOptions options;

    void initSDK() {
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;

        Aws::InitAPI(options);
    }

    void cleanUpSDK() {
        Aws::ShutdownAPI(options);
    }

    void S3Wrapper::listKeys(std::string bucketName) {

        Aws::Client::ClientConfiguration conf;
        conf.region = "eu-west-1";
        Aws::S3::S3Client s3_client(conf);

        Aws::S3::Model::ListObjectsRequest objects_request;
        objects_request.WithBucket(Aws::String(bucketName));

        auto list_objects_outcome = s3_client.ListObjects(objects_request);

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
