#include <catch/catch.hpp>
#include <wasm/FunctionLoader.h>

#include <util/bytes.h>
#include <util/environment.h>
#include <util/func.h>
#include <util/files.h>
#include <util/random.h>
#include <wasm/FileserverFunctionLoader.h>

namespace tests {
    void checkResult(const std::string &filePath, const std::string &expected, const std::vector<uint8_t> &actualBytes) {
        std::vector<uint8_t> actualBytesFromFile = util::readFileToBytes(filePath);

        REQUIRE(actualBytesFromFile == actualBytes);

        std::string actual = util::bytesToString(actualBytes);
        REQUIRE(actual == expected);
    }

    TEST_CASE("Test function round trip", "[wasm]") {
        wasm::FunctionLoader &loader = wasm::getFunctionLoader();
        std::string inputData = "foobarbaz" + std::to_string(util::randomInteger(0, 1000));

        message::Message call;
        call.set_user("test");
        call.set_function("junk");
        call.set_inputdata(inputData);

        std::string filePath;
        std::vector<uint8_t> actualBytes;
        SECTION("Function file") {
            loader.uploadFunction(call);
            filePath = util::getFunctionFile(call);
            actualBytes = loader.loadFunctionBytes(call);
        }

        SECTION("Object file") {
            std::vector<uint8_t> bytes = util::stringToBytes(inputData);
            loader.uploadObjectBytes(call, bytes);
            filePath = util::getFunctionObjectFile(call);
            actualBytes = loader.loadFunctionObjectBytes(call);
        }

        SECTION("Python function") {
            loader.uploadPythonFunction(call);
            filePath = util::getPythonFunctionFile(call);
            actualBytes = loader.loadPythonFunction(call);
        }

        checkResult(filePath, inputData, actualBytes);
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
