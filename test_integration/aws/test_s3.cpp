#include <catch/catch.hpp>

#include <aws/S3Wrapper.h>

using namespace awswrapper;

namespace tests {
    std::string testBucketName = "faasm-test";

    TEST_CASE("Test add/remove keys in bucket", "[aws]") {
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();
        
        s3.addKey(testBucketName, "alpha", "alphaContents");
        s3.addKey(testBucketName, "beta", "betaContents");

        std::vector<std::string> actual = s3.listKeys(testBucketName);
        std::sort(actual.begin(), actual.end());
        std::vector<std::string> expected = {"alpha", "beta"};
        REQUIRE(actual == expected);

        REQUIRE(s3.getKey(testBucketName, "alpha") == "alphaContents");
        REQUIRE(s3.getKey(testBucketName, "beta") == "betaContents");
    }
}