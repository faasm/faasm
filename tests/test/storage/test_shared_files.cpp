#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <boost/filesystem.hpp>

#include <conf/FaasmConfig.h>
#include <faabric/util/files.h>
#include <storage/FileLoader.h>
#include <storage/SharedFiles.h>

using namespace storage;

namespace tests {

class SharedFilesTestFixture : public S3TestFixture
{
  public:
    SharedFilesTestFixture()
      : loader(storage::getFileLoader())
    {
        SharedFiles::clear();
    }

    ~SharedFilesTestFixture() { SharedFiles::clear(); }

  protected:
    storage::FileLoader& loader;
};

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
}
