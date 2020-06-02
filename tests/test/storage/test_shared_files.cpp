#include <catch/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>

#include <storage/SharedFiles.h>
#include <util/files.h>

using namespace storage;

namespace tests {
    TEST_CASE("Check sync shared file", "[storage]") {
        SharedFiles::clear();

        util::SystemConfig &conf = util::getSystemConfig();
        std::string relPath = "shared_test_dir/dummy_file.txt";
        std::string sharedPath = "faasm://" + relPath;
        std::string realPath = util::getSharedFileFile(relPath);
        
        std::string syncedPath;
        std::string overridePath;

        SECTION("Default location") {
            syncedPath = conf.sharedFilesDir + "/" + relPath;
            overridePath = "";
        }

        SECTION("Custom location") {
            overridePath = "/tmp/shared_file_test.txt";
            syncedPath = overridePath;
        }

        std::vector<uint8_t> bytes = {0, 1, 2, 3, 4, 5};

        // Set up file
        if(boost::filesystem::exists(realPath)) {
            boost::filesystem::remove(realPath);
        }
        util::writeBytesToFile(realPath, bytes);

        if(boost::filesystem::exists(syncedPath)) {
            boost::filesystem::remove(syncedPath);
        }
        
        // Do the sync
        SharedFiles::syncSharedFile(sharedPath, overridePath);
        
        // Check the file is now present and contents match
        REQUIRE(boost::filesystem::exists(syncedPath));
        std::vector<uint8_t> actualBytes = util::readFileToBytes(syncedPath);
        REQUIRE(actualBytes == bytes);
    }
}
