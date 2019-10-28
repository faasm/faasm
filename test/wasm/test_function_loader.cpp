#include <catch/catch.hpp>
#include <storage/FileLoader.h>

#include <util/bytes.h>
#include <util/environment.h>
#include <util/func.h>
#include <util/files.h>
#include <storage/FileserverFileLoader.h>
#include <util/config.h>

namespace tests {
    void checkResult(const std::string &filePath, const std::vector<uint8_t> &expected,
                     const std::vector<uint8_t> &actualBytes) {
        std::vector<uint8_t> actualBytesFromFile = util::readFileToBytes(filePath);

        REQUIRE(actualBytesFromFile == actualBytes);
        REQUIRE(actualBytes == expected);
    }

    TEST_CASE("Test function round trip", "[wasm]") {
        storage::FileLoader &loader = storage::getFileLoader();

        message::Message call = util::messageFactory("test", "junk");

        std::string filePath;
        std::vector<uint8_t> actualBytes;
        std::vector<uint8_t> expectedBytes;

        SECTION("Function file") {
            // Need to upload valid data so use real function file
            message::Message realMsg = util::messageFactory("demo", "x2");
            const std::string realFuncFile = util::getFunctionFile(realMsg);

            expectedBytes = util::readFileToBytes(realFuncFile);
            call.set_inputdata(util::bytesToString(expectedBytes));

            loader.uploadFunction(call);

            filePath = util::getFunctionFile(call);
            actualBytes = loader.loadFunctionWasm(call);
        }

        SECTION("Object file") {
            expectedBytes = {8, 7, 6, 5, 4};

            loader.uploadFunctionObjectFile(call, expectedBytes);

            filePath = util::getFunctionObjectFile(call);
            actualBytes = loader.loadFunctionObjectFile(call);
        }

        checkResult(filePath, expectedBytes, actualBytes);
    }

    TEST_CASE("Test invalid storage mode", "[wasm]") {
        const std::string original = util::setEnvVar("FUNCTION_STORAGE", "junk");
        util::SystemConfig &conf = util::getSystemConfig();
        conf.reset();

        REQUIRE_THROWS(storage::getFileLoader());

        util::setEnvVar("FUNCTION_STORAGE", original);
        conf.reset();
    }

    TEST_CASE("Test fileserver function loader requires fileserver URL", "[wasm]") {
        // Instantiate with no url set
        const std::string originalDir = util::setEnvVar("FUNCTION_STORAGE", "fileserver");
        util::unsetEnvVar("FILESERVER_URL");
        util::SystemConfig &conf = util::getSystemConfig();
        conf.reset();

        REQUIRE_THROWS(storage::getFileLoader());

        // Set up a URL
        util::setEnvVar("FILESERVER_URL", "www.foo.com");
        conf.reset();

        // Check no error
        auto loader = (storage::FileserverFileLoader &) storage::getFileLoader();
        REQUIRE(loader.getFileserverUrl() == "www.foo.com");

        util::setEnvVar("FUNCTION_STORAGE", originalDir);
        util::unsetEnvVar("FILESERVER_URL");
        conf.reset();
    }
}
