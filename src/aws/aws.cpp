#include "aws.h"

#include <aws/core/Region.h>
#include <aws/s3/S3Client.h>

#include <util/logging.h>

/**
 * Examples: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/programming-services.html
 *
 * Authentication docs: https://docs.aws.amazon.com/sdk-for-cpp/v1/developer-guide/credentials.html
 */
namespace awswrapper {

    static Aws::SDKOptions options;

    Aws::Client::ClientConfiguration getClientConf(long timeout) {
        Aws::Client::ClientConfiguration conf;
        conf.region = Aws::Region::EU_WEST_1;
        conf.requestTimeoutMs = timeout;

        return conf;
    }

    void initSDK() {
        options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;

        Aws::InitAPI(options);
    }

    void cleanUpSDK() {
        Aws::ShutdownAPI(options);
    }
}
