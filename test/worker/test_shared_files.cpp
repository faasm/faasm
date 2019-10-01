#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <util/config.h>

#include <boost/filesystem.hpp>
#include <util/bytes.h>
#include <util/files.h>
#include <storage/SharedFilesManager.h>


namespace tests {
    TEST_CASE("Test accessing shared files from wasm", "[worker]") {
        cleanSystem();

        // Set up a dummy file
        std::string relativePath = std::string(SHARED_FILE_PREFIX) + "/test/shared-wasm.txt";
        std::string fullPath = util::getSharedFileFile(relativePath);
        if (boost::filesystem::exists(fullPath)) {
            boost::filesystem::remove(fullPath);
        }

        // Enter some data
        std::string expected = "I am some test content\r";
        util::writeBytesToFile(fullPath, util::stringToBytes(expected));

        // Set up the function
        message::Message call = util::messageFactory("demo", "shared_file");
        call.set_inputdata(relativePath);

        std::string actual = execFunctionWithStringResult(call);
        REQUIRE(actual == expected);
    }
}
