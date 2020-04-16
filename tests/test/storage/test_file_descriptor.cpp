#include <catch/catch.hpp>

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <storage/FileLoader.h>
#include <util/files.h>
#include <WAVM/WASI/WASIABI.h>

using namespace storage;

namespace tests {
    FileDescriptor &getRootFd() {
        FileSystem fs;
        fs.prepareFilesystem();

        FileDescriptor &rootFd = fs.getFileDescriptor(4);
        return rootFd;
    }

    TEST_CASE("Check stat and mkdir", "[storage]") {
        FileDescriptor &fd = getRootFd();

        std::string dummyDir = "fs_test_dir";

        util::SystemConfig &conf = util::getSystemConfig();
        std::string realDir = conf.runtimeFilesDir + "/" + dummyDir;
        if (boost::filesystem::exists(realDir)) {
            boost::filesystem::remove_all(realDir);
        }

        // Stat non-existent dir
        Stat dirStat = fd.stat(dummyDir);
        REQUIRE(dirStat.wasiErrno == __WASI_ENOENT);
        REQUIRE(dirStat.failed);

        // Create the directory
        fd.mkdir(dummyDir);
        Stat dirStatB = fd.stat(dummyDir);
        REQUIRE(dirStatB.wasiErrno == 0);
        REQUIRE(!dirStatB.failed);
        REQUIRE(dirStatB.wasiFiletype == __WASI_FILETYPE_DIRECTORY);

        // Delete the directory
        fd.rmdir(dummyDir);
        Stat dirStatC = fd.stat(dummyDir);
        REQUIRE(dirStatC.wasiErrno == __WASI_ENOENT);
        REQUIRE(dirStatC.failed);
    }

    TEST_CASE("Check creating, renaming and deleting a file", "[storage]") {
        FileSystem fs;
        fs.prepareFilesystem();

        int rootFd = 4;
        FileDescriptor &rootFileDesc = fs.getFileDescriptor(4);

        std::string dummyDir = "fs_test_dir";
        std::string dummyPath = dummyDir + "/dummy_file.txt";

        // Set up the directory
        util::SystemConfig &conf = util::getSystemConfig();
        std::string realDir = conf.runtimeFilesDir + "/" + dummyDir;
        if (!boost::filesystem::exists(realDir)) {
            boost::filesystem::create_directories(realDir);
        }

        // Remove the file
        std::string realPath = conf.runtimeFilesDir + "/" + dummyPath;
        if (boost::filesystem::exists(realPath)) {
            boost::filesystem::remove(realPath);
        }

        // Stat the file to begin with
        Stat fileStat = rootFileDesc.stat(dummyPath);
        REQUIRE(fileStat.wasiErrno == __WASI_ENOENT);
        REQUIRE(fileStat.failed);

        // Create the file (ignore perms)
        int fileFd = fs.openFileDescriptor(rootFd, dummyPath, 0, 0, 0,
                                           __WASI_O_CREAT, 0);
        REQUIRE(fileFd > 0);

        FileDescriptor &fileFileDesc = fs.getFileDescriptor(fileFd);

        // Check it now exists
        Stat fileStatB = rootFileDesc.stat(dummyPath);
        REQUIRE(fileStatB.wasiErrno == 0);
        REQUIRE(!fileStatB.failed);
        REQUIRE(fileStatB.wasiFiletype == __WASI_FILETYPE_REGULAR_FILE);

        // Rename it
        std::string newPath = dummyDir + "/renamed_file.txt";
        fileFileDesc.rename(newPath);

        // Stat new and old
        Stat fileStatC = rootFileDesc.stat(newPath);
        REQUIRE(fileStatC.wasiErrno == 0);
        REQUIRE(!fileStatC.failed);
        REQUIRE(fileStatB.wasiFiletype == __WASI_FILETYPE_REGULAR_FILE);

        Stat fileStatD = rootFileDesc.stat(dummyPath);
        REQUIRE(fileStatD.wasiErrno == __WASI_ENOENT);
        REQUIRE(fileStatD.failed);

        // Now remove it and check
        rootFileDesc.unlink(newPath);

        Stat fileStatE = rootFileDesc.stat(newPath);
        REQUIRE(fileStatE.wasiErrno == __WASI_ENOENT);
        REQUIRE(fileStatE.failed);

        Stat fileStatF = rootFileDesc.stat(dummyPath);
        REQUIRE(fileStatF.wasiErrno == __WASI_ENOENT);
        REQUIRE(fileStatF.failed);
    }

    TEST_CASE("Check seek", "[storage]") {
        FileSystem fs;
        fs.prepareFilesystem();

        int rootFd = 4;
        FileDescriptor &rootFileDesc = fs.getFileDescriptor(4);
        std::string dummyPath = "dummy_test_file.txt";

        // Remove the file
        util::SystemConfig &conf = util::getSystemConfig();
        std::string realPath = conf.runtimeFilesDir + "/" + dummyPath;
        if (boost::filesystem::exists(realPath)) {
            boost::filesystem::remove(realPath);
        }

        // Create with some bytes
        std::vector<uint8_t> contents = {0, 1, 2, 3, 4, 5, 6};
        util::writeBytesToFile(realPath, contents);

        int newFd = fs.openFileDescriptor(rootFd, dummyPath, 0, 0, 0, __WASI_O_CREAT, 0);
        FileDescriptor &newFileDesc = fs.getFileDescriptor(newFd);

        // Check zero initially
        REQUIRE(newFileDesc.tell() == 0);

        // Set to zero and check tell still at zero
        uint64_t actual = 0;
        newFileDesc.seek(0, __WASI_WHENCE_CUR, &actual);
        REQUIRE(actual == 0);
        REQUIRE(newFileDesc.tell() == 0);

        // Skip forward
        newFileDesc.seek(2, __WASI_WHENCE_CUR, &actual);
        REQUIRE(actual == 2);
        REQUIRE(newFileDesc.tell() == 2);

        // Skip forward again
        newFileDesc.seek(1, __WASI_WHENCE_CUR, &actual);
        REQUIRE(actual == 3);
        REQUIRE(newFileDesc.tell() == 3);

        // Skip back
        newFileDesc.seek(1, __WASI_WHENCE_SET, &actual);
        REQUIRE(actual == 1);
        REQUIRE(newFileDesc.tell() == 1);
    }
}
