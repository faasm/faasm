#include <catch/catch.hpp>

#include <util/config.h>
#include <util/func.h>
#include <util/files.h>
#include <aws/S3Wrapper.h>
#include <wasm/FunctionLoader.h>

using namespace awswrapper;

namespace tests {
    TEST_CASE("Test read/write keys in bucket", "[aws]") {
        util::SystemConfig &conf = util::getSystemConfig();
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        std::string dataA("£5^Itej__\n\tFoo");
        std::string dataB("ABCDEFGH123$$$&\r*");

        std::vector<uint8_t> byteDataA = {0, 1, 2, 3, 'c', '\\', '@', '$', '%'};
        std::vector<uint8_t> byteDataB = {11, 99, 123, '#', '\n', '\t'};

        SECTION("Test string read/ write") {
            s3.addKeyStr(conf.bucketName, "alpha", dataA);
            s3.addKeyStr(conf.bucketName, "beta", dataB);

            REQUIRE(s3.getKeyStr(conf.bucketName, "alpha") == dataA);
            REQUIRE(s3.getKeyStr(conf.bucketName, "beta") == dataB);
        }

        SECTION("Test listing keys") {
            s3.addKeyStr(conf.bucketName, "alpha", dataA);
            s3.addKeyStr(conf.bucketName, "beta", dataB);

            std::vector<std::string> actual = s3.listKeys(conf.bucketName);
            std::sort(actual.begin(), actual.end());
            std::vector<std::string> expected = {"alpha", "beta"};

            REQUIRE(actual == expected);
        }

        SECTION("Test byte read/write") {
            s3.addKeyBytes(conf.bucketName, "alpha", byteDataA);
            s3.addKeyBytes(conf.bucketName, "beta", byteDataB);

            const std::vector<uint8_t> actualA = s3.getKeyBytes(conf.bucketName, "alpha");
            const std::vector<uint8_t> actualB = s3.getKeyBytes(conf.bucketName, "beta");

            REQUIRE(actualA == byteDataA);
            REQUIRE(actualB == byteDataB);
        }

        s3.deleteKey(conf.bucketName, "alpha");
        s3.deleteKey(conf.bucketName, "beta");

        std::vector<std::string> actualEmpty = s3.listKeys(conf.bucketName);
        REQUIRE(actualEmpty.empty());
    }

    TEST_CASE("Test read/write function data in bucket") {
        util::SystemConfig &conf = util::getSystemConfig();
        S3Wrapper &s3 = S3Wrapper::getThreadLocal();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("echo");

        // Wasm data is uploaded via the message input data
        const std::string &funcPath = util::getFunctionFile(msg);
        const std::vector<uint8_t> &wasmBytes = util::readFileToBytes(funcPath);
        msg.set_inputdata(wasmBytes.data(), wasmBytes.size());

        // Object files are uploaded by passing bytes directly
        const std::string &objPath = util::getFunctionObjectFile(msg);
        const std::vector<uint8_t> &objectBytes = util::readFileToBytes(objPath);

        wasm::FunctionLoader &loader = wasm::getFunctionLoader();

        // Do both uploads
        loader.uploadFunction(msg);
        loader.uploadObjectBytes(msg, objectBytes);
        
        // Download again
        const std::vector<uint8_t> &actualWasmBytes = loader.loadFunctionBytes(msg);
        const std::vector<uint8_t> &actualObjectBytes = loader.loadFunctionObjectBytes(msg);
        REQUIRE(actualWasmBytes == wasmBytes);
        REQUIRE(actualObjectBytes == objectBytes);

        s3.deleteKey(conf.bucketName, funcPath);
        s3.deleteKey(conf.bucketName, objPath);

        std::vector<std::string> actualEmpty = s3.listKeys(conf.bucketName);
        REQUIRE(actualEmpty.empty());
    }
}