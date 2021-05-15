#include <catch2/catch.hpp>
#include <storage/FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <storage/FileserverFileLoader.h>

namespace tests {
void checkResult(const std::string& filePath,
                 const std::vector<uint8_t>& expected,
                 const std::vector<uint8_t>& actualBytes)
{
    std::vector<uint8_t> actualBytesFromFile =
      faabric::util::readFileToBytes(filePath);

    REQUIRE(actualBytesFromFile == actualBytes);
    REQUIRE(actualBytes == expected);
}

TEST_CASE("Test function round trip", "[wasm]")
{
    storage::FileLoader& loader = storage::getFileLoader();

    faabric::Message call = faabric::util::messageFactory("test", "junk");

    std::string filePath;
    std::vector<uint8_t> actualBytes;
    std::vector<uint8_t> expectedBytes;

    SECTION("Function file")
    {
        // Need to upload valid data so use real function file
        faabric::Message realMsg = faabric::util::messageFactory("demo", "x2");
        const std::string realFuncFile = conf::getFunctionFile(realMsg);

        expectedBytes = faabric::util::readFileToBytes(realFuncFile);
        call.set_inputdata(faabric::util::bytesToString(expectedBytes));

        loader.uploadFunction(call);

        filePath = conf::getFunctionFile(call);
        actualBytes = loader.loadFunctionWasm(call);
    }

    SECTION("Object file")
    {
        expectedBytes = { 8, 7, 6, 5, 4 };

        loader.uploadFunctionObjectFile(call, expectedBytes);

        filePath = conf::getFunctionObjectFile(call);
        actualBytes = loader.loadFunctionObjectFile(call);
    }

    checkResult(filePath, expectedBytes, actualBytes);
}

TEST_CASE("Test invalid storage mode", "[wasm]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    conf.functionStorage = "junk";

    REQUIRE_THROWS(storage::getFileLoader());

    conf.reset();
}

TEST_CASE("Test fileserver function loader requires fileserver URL", "[wasm]")
{
    // Instantiate with no url set
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    conf.functionStorage = "fileserver";
    conf.fileserverUrl = "";

    REQUIRE_THROWS(storage::getFileLoader());

    // Set up a URL
    conf.fileserverUrl = "www.foo.com";

    // Check no error
    auto loader = (storage::FileserverFileLoader&)storage::getFileLoader();
    REQUIRE(loader.getFileserverUrl() == "www.foo.com");

    conf.reset();
}
}
