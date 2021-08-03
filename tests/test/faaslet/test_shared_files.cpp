#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <storage/FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <boost/filesystem.hpp>

namespace tests {
TEST_CASE_METHOD(SharedFilesTestFixture,
                 "Test accessing shared files from wasm",
                 "[faaslet]")
{
    // Set up a dummy file location
    std::string relativePath = "test/shared-wasm.txt";
    std::string fullPath = loader.getSharedFileFile(relativePath);
    REQUIRE(!boost::filesystem::exists(fullPath));

    // Upload some data
    std::string expected = "I am some test content\r";
    std::vector<uint8_t> expectedBytes = faabric::util::stringToBytes(expected);
    loader.uploadSharedFile(relativePath, expectedBytes);

    // Check file now exists, then delete
    REQUIRE(boost::filesystem::exists(fullPath));
    boost::filesystem::remove(fullPath);

    // Set up the function
    faabric::Message call =
      faabric::util::messageFactory("demo", "shared_file");
    std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
    call.set_inputdata(sharedPath);

    // Invoke the function and make sure result is echoing the file content
    std::string actual = execFunctionWithStringResult(call);
    REQUIRE(actual == expected);

    // Check file has been synced locally
    REQUIRE(boost::filesystem::exists(fullPath));
}
}
