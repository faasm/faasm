#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>

#include <boost/filesystem.hpp>
#include <util/bytes.h>
#include <util/files.h>


namespace tests {
    TEST_CASE("Test accessing shared files from wasm", "[faaslet]") {
        cleanSystem();

        // Set up a dummy file
        std::string relativePath = "test/shared-wasm.txt";
        std::string fullPath = util::getSharedFileFile(relativePath);
        if (boost::filesystem::exists(fullPath)) {
            boost::filesystem::remove(fullPath);
        }

        // Enter some data
        std::string expected = "I am some test content\r";
        util::writeBytesToFile(fullPath, util::stringToBytes(expected));

        // Set up the function
        message::Message call = util::messageFactory("demo", "shared_file");
        std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
        call.set_inputdata(sharedPath);

        std::string actual = execFunctionWithStringResult(call);
        REQUIRE(actual == expected);
    }
}
