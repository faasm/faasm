#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <boost/filesystem.hpp>

#include <conf/FaasmConfig.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>

using namespace storage;

namespace tests {

TEST_CASE_METHOD(SharedFilesTestFixture, "Check sync shared file", "[storage]")
{
    std::string relPath = "shared_test_dir/dummy_file.txt";
    std::string sharedPath = "faasm://" + relPath;

    std::string defaultSyncedPath = loader.getSharedFileFile(relPath);
    std::string syncedPath;
    std::string overridePath;

    SECTION("Default location")
    {
        syncedPath = defaultSyncedPath;
        overridePath = "";
    }

    SECTION("Custom location")
    {
        overridePath = "/tmp/shared_file_test.txt";
        syncedPath = overridePath;
    }

    std::vector<uint8_t> bytes = { 0, 1, 2, 3, 4, 5 };

    // Upload the file
    loader.uploadSharedFile(relPath, bytes);

    // Clear up local files
    boost::filesystem::remove(defaultSyncedPath);
    boost::filesystem::remove(syncedPath);

    // Do the sync
    SharedFiles::syncSharedFile(sharedPath, overridePath);

    // Check the file is now present and contents match
    REQUIRE(boost::filesystem::exists(syncedPath));
    std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(syncedPath);
    REQUIRE(actualBytes == bytes);
}

TEST_CASE_METHOD(SharedFilesTestFixture, "Check sync python file", "[storage]")
{
    // Upload a python function
    std::string user = "alpha";
    std::string function = "beta";
    std::vector<uint8_t> contents = { 0, 1, 2, 3, 4 };

    faabric::Message msg;
    msg.set_ispython(true);
    msg.set_pythonuser(user);
    msg.set_pythonfunction(function);
    msg.set_inputdata(contents.data(), contents.size());

    // Prepare paths
    std::string sharedFilePath = loader.getPythonFunctionFile(msg);
    std::string runtimeFilePath =
      fmt::format("{}/{}",
                  faasmConf.runtimeFilesDir,
                  loader.getPythonFunctionRelativePath(msg));

    // Ensure files don't exist already
    boost::filesystem::remove(sharedFilePath);
    boost::filesystem::remove(runtimeFilePath);

    // Upload the function
    loader.uploadPythonFunction(msg);

    // Check the shared file is created, but not the runtime one
    REQUIRE(boost::filesystem::exists(sharedFilePath));
    REQUIRE(!boost::filesystem::exists(runtimeFilePath));

    // Delete the local shared file
    boost::filesystem::remove(sharedFilePath);

    // Sync as a shared file
    SharedFiles::syncPythonFunctionFile(msg);

    // Check both files now in place
    REQUIRE(boost::filesystem::exists(sharedFilePath));
    REQUIRE(boost::filesystem::exists(runtimeFilePath));

    std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(runtimeFilePath);
    REQUIRE(actualBytes.size() == contents.size());
    REQUIRE(actualBytes == contents);
}
}
