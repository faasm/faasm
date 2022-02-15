#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <storage/FileLoader.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <boost/filesystem.hpp>

namespace tests {

class LocalFixture
  : public SharedFilesTestFixture
  , public ExecutorContextTestFixture
{};

TEST_CASE_METHOD(LocalFixture, "Test accessing shared files from wasm", "[faaslet]")
{
    // Set up a dummy file location
    std::string relativePath = "test/shared-wasm.txt";
    std::string fullPath = loader.getSharedFileFile(relativePath);
    boost::filesystem::remove(fullPath);

    // Upload some data
    std::string expected = "I am some test content\r";
    std::vector<uint8_t> expectedBytes = faabric::util::stringToBytes(expected);
    loader.uploadSharedFile(relativePath, expectedBytes);

    // Check file now exists, then delete
    REQUIRE(boost::filesystem::exists(fullPath));
    boost::filesystem::remove(fullPath);

    // Set up the function
    auto req = setUpContext("demo", "shared_file");
    faabric::Message& msg = req->mutable_messages()->at(0);
    std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
    msg.set_inputdata(sharedPath);

    // Invoke the function and make sure result is echoing the file content
    std::string actual = execFunctionWithStringResult(msg);
    REQUIRE(actual == expected);

    // Check file has been synced locally
    REQUIRE(boost::filesystem::exists(fullPath));
}
}
