#include <catch/catch.hpp>
#include <storage/FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/files.h>
#include <storage/FileserverFileLoader.h>
#include <faabric/util/config.h>

namespace tests {
    void checkResult(const std::string &filePath, const std::vector<uint8_t> &expected,
                     const std::vector<uint8_t> &actualBytes) {
        std::vector<uint8_t> actualBytesFromFile = faabric::utilreadFileToBytes(filePath);

        REQUIRE(actualBytesFromFile == actualBytes);
        REQUIRE(actualBytes == expected);
    }

    TEST_CASE("Test function round trip", "[wasm]") {
        storage::FileLoader &loader = storage::getFileLoader();

        faabric::Message call = faabric::utilmessageFactory("test", "junk");

        std::string filePath;
        std::vector<uint8_t> actualBytes;
        std::vector<uint8_t> expectedBytes;

        SECTION("Function file") {
            // Need to upload valid data so use real function file
            faabric::Message realMsg = faabric::utilmessageFactory("demo", "x2");
            const std::string realFuncFile = faabric::utilgetFunctionFile(realMsg);

            expectedBytes = faabric::utilreadFileToBytes(realFuncFile);
            call.set_inputdata(faabric::utilbytesToString(expectedBytes));

            loader.uploadFunction(call);

            filePath = faabric::utilgetFunctionFile(call);
            actualBytes = loader.loadFunctionWasm(call);
        }

        SECTION("Object file") {
            expectedBytes = {8, 7, 6, 5, 4};

            loader.uploadFunctionObjectFile(call, expectedBytes);

            filePath = faabric::utilgetFunctionObjectFile(call);
            actualBytes = loader.loadFunctionObjectFile(call);
        }

        checkResult(filePath, expectedBytes, actualBytes);
    }

    TEST_CASE("Test invalid storage mode", "[wasm]") {
        const std::string original = faabric::utilsetEnvVar("FUNCTION_STORAGE", "junk");
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();
        conf.reset();

        REQUIRE_THROWS(storage::getFileLoader());

        faabric::utilsetEnvVar("FUNCTION_STORAGE", original);
        conf.reset();
    }

    TEST_CASE("Test fileserver function loader requires fileserver URL", "[wasm]") {
        // Instantiate with no url set
        const std::string originalDir = faabric::utilsetEnvVar("FUNCTION_STORAGE", "fileserver");
        faabric::utilunsetEnvVar("FILESERVER_URL");
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();
        conf.reset();

        REQUIRE_THROWS(storage::getFileLoader());

        // Set up a URL
        faabric::utilsetEnvVar("FILESERVER_URL", "www.foo.com");
        conf.reset();

        // Check no error
        auto loader = (storage::FileserverFileLoader &) storage::getFileLoader();
        REQUIRE(loader.getFileserverUrl() == "www.foo.com");

        faabric::utilsetEnvVar("FUNCTION_STORAGE", originalDir);
        faabric::utilunsetEnvVar("FILESERVER_URL");
        conf.reset();
    }
}
