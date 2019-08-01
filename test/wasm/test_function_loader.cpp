#include <catch/catch.hpp>
#include <wasm/FunctionLoader.h>

#include <util/bytes.h>
#include <util/environment.h>
#include <util/func.h>
#include <util/files.h>
#include <wasm/FileserverFunctionLoader.h>

namespace tests {
    void checkResult(const std::string &filePath, const std::vector<uint8_t> &expected, const std::vector<uint8_t> &actualBytes) {
        std::vector<uint8_t> actualBytesFromFile = util::readFileToBytes(filePath);

        REQUIRE(actualBytesFromFile == actualBytes);
        REQUIRE(actualBytes== expected);
    }

    TEST_CASE("Test function round trip", "[wasm]") {
        wasm::FunctionLoader &loader = wasm::getFunctionLoader();

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
            actualBytes = loader.loadFunctionBytes(call);
        }

        SECTION("Python function") {
            // Python file can be junk
            expectedBytes = {1, 2, 3, 4, 5};

            call.set_inputdata(util::bytesToString(expectedBytes));
            loader.uploadPythonFunction(call);

            filePath = util::getPythonFunctionFile(call);
            actualBytes = loader.loadPythonFunction(call);
        }

        SECTION("Object file") {
            expectedBytes = {8, 7, 6, 5, 4};

            loader.uploadObjectBytes(call, expectedBytes);

            filePath = util::getFunctionObjectFile(call);
            actualBytes = loader.loadFunctionObjectBytes(call);
        }

        checkResult(filePath, expectedBytes, actualBytes);
    }

    TEST_CASE("Test invalid storage mode", "[wasm]") {
        util::setEnvVar("FUNCTION_STORAGE", "junk");
        util::SystemConfig &conf = util::getSystemConfig();
        conf.reset();

        REQUIRE_THROWS(wasm::getFunctionLoader());

        util::unsetEnvVar("FUNCTION_STORAGE");
        conf.reset();
    }

    TEST_CASE("Test fileserver function loader requires fileserver URL", "[wasm]") {
        // Instantiate with no url set
        util::setEnvVar("FUNCTION_STORAGE", "fileserver");
        util::unsetEnvVar("FILESERVER_URL");
        util::SystemConfig &conf = util::getSystemConfig();
        conf.reset();

        REQUIRE_THROWS(wasm::getFunctionLoader());

        // Set up a URL
        util::setEnvVar("FILESERVER_URL", "www.foo.com");
        conf.reset();

        // Check no error
        auto loader = (wasm::FileserverFunctionLoader&) wasm::getFunctionLoader();
        REQUIRE(loader.getFileserverUrl() == "www.foo.com");

        util::unsetEnvVar("FILESERVER_URL");
        util::unsetEnvVar("FUNCTION_STORAGE");
        conf.reset();
    }
}
