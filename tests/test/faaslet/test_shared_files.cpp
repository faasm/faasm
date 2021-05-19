#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <conf/function_utils.h>

#include <boost/filesystem.hpp>

namespace tests {
TEST_CASE("Test accessing shared files from wasm", "[faaslet]")
{
    cleanSystem();

    // Set up a dummy file
    std::string relativePath = "test/shared-wasm.txt";
    std::string fullPath = conf::getSharedFileFile(relativePath);
    if (boost::filesystem::exists(fullPath)) {
        boost::filesystem::remove(fullPath);
    }

    // Enter some data
    std::string expected = "I am some test content\r";
    faabric::util::writeBytesToFile(fullPath,
                                    faabric::util::stringToBytes(expected));

    // Set up the function
    faabric::Message call =
      faabric::util::messageFactory("demo", "shared_file");
    std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
    call.set_inputdata(sharedPath);

    std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == expected);
}
}
