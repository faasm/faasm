#include <catch/catch.hpp>

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <storage/SharedFilesManager.h>
#include <boost/filesystem.hpp>
#include <storage/FileLoader.h>
#include <util/files.h>

using namespace storage;

namespace tests {
    TEST_CASE("Check loading local files", "[storage]") {
        SharedFilesManager &sfm = getSharedFilesManager();
        sfm.clear();

        std::string validPath = "include/python3.7m/Python.h";
        std::string maskedValidPath = prependRuntimeRoot(validPath);
        std::string invalidPath = "/foobar/123/blah";

        SECTION("Stat valid file") {
            // First stat the file
            struct stat64 sfmStat{};
            sfm.statFile(validPath, &sfmStat);

            // Stat natively
            struct stat64 nativeStat{};
            stat64(maskedValidPath.c_str(), &nativeStat);

            // Check they're equal
            REQUIRE(sfmStat.st_size == nativeStat.st_size);
        }

        SECTION("Open valid file") {
            // Open the file
            int fd = sfm.openFile(validPath, O_RDONLY, 0);
            REQUIRE(fd > 0);

            // Read in from the returned descriptor
            std::vector<char> actual(100);
            read(fd, actual.data(), 100);

            // Read in from the actual file
            std::vector<char> expected(100);
            FILE *fp = fopen(maskedValidPath.c_str(), "r");
            fread(expected.data(), 100, 1, fp);

            REQUIRE(actual == expected);
        }

        SECTION("Stat non-existent") {
            struct stat64 s{};
            int fd = sfm.statFile(invalidPath, &s);
            bool isBlocked = (fd == -EPERM) || (fd == -ENOENT);
            REQUIRE(isBlocked);
        }

        SECTION("Open non-existent") {
            int fd = sfm.openFile(invalidPath, O_RDONLY, 0);
            REQUIRE(fd == -ENOENT);
        }
    }

    TEST_CASE("Check loading shared files", "[storage]") {
        SharedFilesManager &sfm = getSharedFilesManager();
        sfm.clear();

        util::SystemConfig &conf = util::getSystemConfig();

        std::vector<uint8_t> expectedBytes = {6, 5, 4, 0, 1};
        std::string relativePath = "test/valid_shared_file.txt";
        std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
        bool valid;

        // Prepare paths for both copies of file
        std::string storagePath = conf.sharedFilesStorageDir + "/" + relativePath;
        std::string usedPath = conf.sharedFilesDir + "/" + relativePath;

        // Clear up beforehand
        if (boost::filesystem::exists(storagePath)) {
            boost::filesystem::remove(storagePath);
        }

        if (boost::filesystem::exists(usedPath)) {
            boost::filesystem::remove(usedPath);
        }

        SECTION("Valid shared file") {
            FileLoader &loader = storage::getFileLoader();
            loader.uploadSharedFile(relativePath, expectedBytes);
            valid = true;
        }

        SECTION("Invalid shared file") {
            valid = false;
        }

        // Stat the file with the SFM and natively
        struct stat64 s{};
        int statRes = sfm.statFile(sharedPath.c_str(), &s);

        struct stat64 nativeS{};
        stat64(storagePath.c_str(), &nativeS);

        if(valid) {
            REQUIRE(statRes == 0);
            REQUIRE(s.st_size == nativeS.st_size);
        } else {
            REQUIRE(statRes == -ENOENT);
        }

        // Open the shared file
        int fd = sfm.openFile(sharedPath, O_RDONLY, 0);

        if (!valid) {
            REQUIRE(fd == -ENOENT);
        } else {
            REQUIRE(fd > 0);

            std::vector<uint8_t> actualStored = util::readFileToBytes(storagePath);
            std::vector<uint8_t> actualUsed = util::readFileToBytes(usedPath);

            // Read from the fd
            std::vector<uint8_t> actual(5);
            read(fd, actual.data(), 5);

            // Check this matches the stored and used versions
            REQUIRE(actual == actualStored);
            REQUIRE(actual == actualUsed);
        }
    }
}
