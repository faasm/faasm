#include <catch/catch.hpp>

#include <aws/aws.h>

using namespace awswrapper;

namespace tests {
    std::string testBucketName = "faasm-test";

    TEST_CASE("Test add/remote keys in bucket", "[s3]") {
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();
        
        s3.addKey(testBucketName, "alpha", "alphaContents");
        s3.addKey(testBucketName, "beta", "betaContents");

        std::vector<std::string> actual = s3.listKeys(testBucketName);
        REQUIRE(actual.size() == 2);

        REQUIRE(s3.getKey(testBucketName, "alpha") == "alphaContents");
        REQUIRE(s3.getKey(testBucketName, "beta") == "betaContents");
    }
}