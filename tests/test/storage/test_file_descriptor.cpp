#include <catch/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>
#include <util/files.h>
#include <WAVM/WASI/WASIABI.h>
#include <storage/SharedFiles.h>

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
        SharedFiles::clear();

        FileSystem fs;
        fs.prepareFilesystem();

        int rootFd = 4;
        FileDescriptor &rootFileDesc = fs.getFileDescriptor(4);

        util::SystemConfig &conf = util::getSystemConfig();
        std::string dummyPath;
        std::string realPath;
        std::string contentPath;

        std::vector<uint8_t> contents = {0, 1, 2, 3, 4, 5, 6};

        SECTION("Local file") {
            dummyPath = "dummy_test_file.txt";
            realPath = conf.runtimeFilesDir + "/" + dummyPath;
            contentPath = realPath;
            util::writeBytesToFile(realPath, contents);
        }

        SECTION("Shared file") {
            dummyPath = "faasm://dummy_test_file.txt";
            contentPath = util::getSharedFileFile("dummy_test_file.txt");

            // This is the path where the file should end up after being synced
            realPath = SharedFiles::realPathForSharedFile(dummyPath);
        }

        // Set up the file
        if (boost::filesystem::exists(realPath)) {
            boost::filesystem::remove(realPath);
        }
        util::writeBytesToFile(contentPath, contents);

        // Open file descriptor for the file
        int newFd = fs.openFileDescriptor(rootFd, dummyPath, 0, 0, 0, __WASI_O_CREAT, 0);
        REQUIRE(newFd > 0);
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

        boost::filesystem::remove(realPath);
    }

    TEST_CASE("Check stat and read shared file", "[storage]") {
        SharedFiles::clear();

        FileSystem fs;
        fs.prepareFilesystem();

        int rootFd = 4;
        FileDescriptor &rootFileDesc = fs.getFileDescriptor(4);

        // Set up the shared file
        std::string relativePath = "test/shared-file-stat.txt";
        std::string fullPath = util::getSharedFileFile(relativePath);
        if (boost::filesystem::exists(fullPath)) {
            boost::filesystem::remove(fullPath);
        }

        std::vector<uint8_t> contents = {0, 1, 2, 3, 4, 5};
        util::writeBytesToFile(fullPath, contents);

        // Stat it as a relative path
        std::string sharedPath = std::string(SHARED_FILE_PREFIX) + relativePath;
        const Stat &statRes = rootFileDesc.stat(sharedPath);
        REQUIRE(!statRes.failed);
        REQUIRE(statRes.wasiErrno == 0);
        REQUIRE(statRes.wasiFiletype == __WASI_FILETYPE_REGULAR_FILE);

        // Open it as a shared file
        int fileFd = fs.openFileDescriptor(rootFd, sharedPath, 0, 0, 0, __WASI_O_CREAT, 0);
        REQUIRE(fileFd > 0);
        FileDescriptor &fileFileDesc = fs.getFileDescriptor(fileFd);

        // Stat again using the absolute path
        const Stat &statResB = fileFileDesc.stat();
        REQUIRE(!statResB.failed);
        REQUIRE(statResB.wasiErrno == 0);
        REQUIRE(statResB.wasiFiletype == __WASI_FILETYPE_REGULAR_FILE);

        // Check path of this new file descriptor
        util::SystemConfig &conf = util::getSystemConfig();
        REQUIRE(fileFileDesc.getPath() == sharedPath);

        const std::string &realPath = storage::SharedFiles::realPathForSharedFile(sharedPath);
        const std::vector<uint8_t> &actualContents = util::readFileToBytes(realPath);
        REQUIRE(actualContents == contents);
    }
}
