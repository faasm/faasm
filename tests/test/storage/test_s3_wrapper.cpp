#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "faasm_fixtures.h"

#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <storage/S3Wrapper.h>

namespace tests {

TEST_CASE_METHOD(S3TestFixture, "Test read/write keys in bucket", "[s3]")
{
    std::string simpleData = "I am a string";
    std::string dataA("£5^Itej__\n\tFoo");
    std::string dataB("ABCDEFGH123$$$&\r*");

    std::vector<uint8_t> byteDataA = { 0, 1, 2, 3, 'c', '\\', '@', '$', '%' };
    std::vector<uint8_t> byteDataB = { 11, 99, 123, '#', '\n', '\t' };

    SECTION("Test list buckets")
    {
        std::vector<std::string> buckets = s3.listBuckets();
        std::vector<std::string> expected = { "faasm", faasmConf.s3Bucket };
        REQUIRE(buckets == expected);

        s3.deleteBucket(faasmConf.s3Bucket);

        std::vector<std::string> expectedAfter = { "faasm" };
        std::vector<std::string> bucketsAfter = s3.listBuckets();
        REQUIRE(bucketsAfter == expectedAfter);
    }

    SECTION("Test simple string read/ write")
    {
        s3.addKeyStr(faasmConf.s3Bucket, "simple", simpleData);
        REQUIRE(s3.getKeyStr(faasmConf.s3Bucket, "simple") == simpleData);
    }

    SECTION("Test string read/ write")
    {
        s3.addKeyStr(faasmConf.s3Bucket, "alpha", dataA);
        s3.addKeyStr(faasmConf.s3Bucket, "beta", dataB);

        REQUIRE(s3.getKeyStr(faasmConf.s3Bucket, "alpha") == dataA);
        REQUIRE(s3.getKeyStr(faasmConf.s3Bucket, "beta") == dataB);
    }

    SECTION("Test listing keys")
    {
        s3.addKeyStr(faasmConf.s3Bucket, "alpha", dataA);
        s3.addKeyStr(faasmConf.s3Bucket, "beta", dataB);

        std::vector<std::string> actual = s3.listKeys(faasmConf.s3Bucket);
        std::sort(actual.begin(), actual.end());
        std::vector<std::string> expected = { "alpha", "beta" };

        REQUIRE(actual == expected);
    }

    SECTION("Test byte read/write")
    {
        s3.addKeyBytes(faasmConf.s3Bucket, "alpha", byteDataA);
        s3.addKeyBytes(faasmConf.s3Bucket, "beta", byteDataB);

        const std::vector<uint8_t> actualA =
          s3.getKeyBytes(faasmConf.s3Bucket, "alpha");
        const std::vector<uint8_t> actualB =
          s3.getKeyBytes(faasmConf.s3Bucket, "beta");

        REQUIRE(actualA == byteDataA);
        REQUIRE(actualB == byteDataB);
    }

    SECTION("Test tolerate missing key")
    {
        const std::vector<uint8_t> actual =
          s3.getKeyBytes(faasmConf.s3Bucket, "blahblah", true);
        REQUIRE(actual.empty());
    }

    SECTION("Test don't tolerate missing key")
    {
        REQUIRE_THROWS(s3.getKeyBytes(faasmConf.s3Bucket, "blahblah"));
    }

    s3.deleteKey(faasmConf.s3Bucket, "alpha");
    s3.deleteKey(faasmConf.s3Bucket, "beta");
    s3.deleteKey(faasmConf.s3Bucket, "simple");

    std::vector<std::string> actualEmpty = s3.listKeys(faasmConf.s3Bucket);
    REQUIRE(actualEmpty.empty());
}
}
