#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>

#include <util/logging.h>
#include <util/environment.h>
#include <aws/aws.h>

int main(int argc, char *argv[]) {
    util::setEnvVar("FUNCTION_STORAGE", "s3");
    util::setEnvVar("GLOBAL_MESSAGE_BUS", "sqs");
    util::setEnvVar("BUCKET_NAME", "faasm-test");
    util::setEnvVar("QUEUE_NAME", "faasm-test");

    util::initLogging();
    awswrapper::initSDK();

    int result = Catch::Session().run(argc, argv);

    awswrapper::cleanUpSDK();

    return result;
}