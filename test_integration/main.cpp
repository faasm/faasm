#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <util/util.h>
#include <aws/aws.h>

int main(int argc, char *argv[]) {
    util::initLogging();
    awswrapper::initSDK();

    int result = Catch::Session().run(argc, argv);

    awswrapper::cleanUpSDK();

    return result;
}