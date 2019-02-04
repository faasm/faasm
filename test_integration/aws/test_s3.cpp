#include <catch/catch.hpp>

#include <util/config.h>
#include <aws/S3Wrapper.h>

using namespace awswrapper;

namespace tests {
    TEST_CASE("Test add/remove keys in bucket", "[aws]") {
        util::SystemConfig &conf = util::getSystemConfig();

        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        s3.addKey(conf.bucketName, "alpha", "alphaContents");
        s3.addKey(conf.bucketName, "beta", "betaContents");

        std::vector<std::string> actual = s3.listKeys(conf.bucketName);
        std::sort(actual.begin(), actual.end());
        std::vector<std::string> expected = {"alpha", "beta"};
        REQUIRE(actual == expected);

        REQUIRE(s3.getKey(conf.bucketName, "alpha") == "alphaContents");
        REQUIRE(s3.getKey(conf.bucketName, "beta") == "betaContents");
    }
}