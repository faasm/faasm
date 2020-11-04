#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>
#include <faabric/util/files.h>
#include <stdlib.h>
#include <storage/LocalFileLoader.h>

using namespace storage;

namespace tests {
TEST_CASE("Check uploading and loading shared files", "[storage]")
{
    std::string relativePath = "test/local_file_loader.txt";
    std::vector<uint8_t> expected = { 1, 5, 3, 2, 4 };

    storage::FileLoader& loader = storage::getFileLoader();
    loader.uploadSharedFile(relativePath, expected);

    const std::vector<uint8_t> actual = loader.loadSharedFile(relativePath);

    REQUIRE(actual == expected);

    // Check it's written where we expect it to be too
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    boost::filesystem::path fullPath(conf.sharedFilesStorageDir);
    fullPath.append(relativePath);

    const std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(fullPath.string());
    REQUIRE(actualBytes == expected);
}
}
