#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {

class S3ExecTestFixture
  : public FunctionExecTestFixture
  , public S3TestFixture
{
  public:
    S3ExecTestFixture()
    {
        faasmConf.s3Bucket = testBucketName;
        s3.createBucket(faasmConf.s3Bucket);
    };

    ~S3ExecTestFixture() {}

    faabric::Message doS3ExecTest(const std::string& function,
                                  const std::string& inputData = "",
                                  const std::string& cmdline = "")
    {
        auto req = faabric::util::batchExecFactory("s3", function, 1);

        if (!inputData.empty()) {
            req->mutable_messages(0)->set_inputdata(inputData);
        }

        if (!cmdline.empty()) {
            req->mutable_messages(0)->set_cmdline(cmdline);
        }

        faabric::Message result = executeWithPool(req).at(0);

        return result;
    }

  protected:
    storage::S3Wrapper s3cli;
    const std::string testBucketName = "faasm-test-s3-exec";
};

TEST_CASE_METHOD(S3ExecTestFixture, "Get number of buckets", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    auto result = doS3ExecTest("get_num_buckets");

    std::string expectedNumBuckets = std::to_string(s3cli.listBuckets().size());

    REQUIRE(result.outputdata() == expectedNumBuckets);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(S3ExecTestFixture, "List buckets", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    auto result = doS3ExecTest("list_buckets");

    std::string expectedNumBuckets = std::to_string(s3cli.listBuckets().size());

    std::vector<std::string> expectedBuckets = s3cli.listBuckets();
    std::vector<std::string> actualBuckets;

    std::string delimiter = "|";
    size_t pos = 0;
    std::string token;
    std::string actualStr = result.outputdata();
    while ((pos = actualStr.find(delimiter)) != std::string::npos) {
        actualBuckets.push_back(actualStr.substr(0, pos));
        actualStr.erase(0, pos + delimiter.length());
    }
    actualBuckets.push_back(actualStr);

    REQUIRE(actualBuckets == expectedBuckets);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(S3ExecTestFixture, "Get number of keys", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    // Add a few keys
    s3cli.addKeyStr(testBucketName, "hello", "world");
    s3cli.addKeyStr(testBucketName, "foo", "bar");

    std::string expectedNumKeys =
      std::to_string(s3cli.listKeys(testBucketName).size());
    auto result = doS3ExecTest("get_num_keys", testBucketName);

    REQUIRE(result.outputdata() == expectedNumKeys);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(S3ExecTestFixture, "List keys", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    // Add a few keys
    s3cli.addKeyStr(testBucketName, "hello", "world");
    s3cli.addKeyStr(testBucketName, "foo", "bar");

    std::vector<std::string> expectedKeys = s3cli.listKeys(testBucketName);
    std::vector<std::string> actualKeys;

    auto result = doS3ExecTest("list_keys", testBucketName);

    std::string delimiter = "|";
    size_t pos = 0;
    std::string token;
    std::string actualStr = result.outputdata();
    while ((pos = actualStr.find(delimiter)) != std::string::npos) {
        actualKeys.push_back(actualStr.substr(0, pos));
        actualStr.erase(0, pos + delimiter.length());
    }
    actualKeys.push_back(actualStr);

    REQUIRE(actualKeys == expectedKeys);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(S3ExecTestFixture, "Add key bytes", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    std::string bytesToAdd = "bar";
    std::string keyName = "foo";
    std::string cmdline = fmt::format("{} {}", testBucketName, keyName);

    auto result = doS3ExecTest("add_key_bytes", bytesToAdd, cmdline);

    auto actualKeyStr = s3cli.getKeyStr(testBucketName, keyName);
    REQUIRE(actualKeyStr == bytesToAdd);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(S3ExecTestFixture, "Get key bytes", "[s3]")
{
    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifndef FAASM_SGX_DISABLED_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    // Test a very large key to stress ECall/OCall bufer size limits
    std::string bytesToAdd(128, 'b');
    std::string keyName = "foo";

    // Add some bytes to the key
    s3cli.addKeyStr(testBucketName, keyName, bytesToAdd);

    std::string cmdline = fmt::format("{} {}", testBucketName, keyName);

    auto result = doS3ExecTest("get_key_bytes", bytesToAdd, cmdline);
    bool outputEqual = result.outputdata() == bytesToAdd;
    REQUIRE(outputEqual);
    REQUIRE(result.returnvalue() == 0);
}
}
