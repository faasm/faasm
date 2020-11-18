#include "faabric/util/bytes.h"
#include "faabric/util/func.h"
#include <boost/filesystem/operations.hpp>
#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>
#include <faabric/util/files.h>
#include <stdlib.h>
#include <storage/LocalFileLoader.h>

#define SHARED_OBJ                                                             \
    "/usr/local/faasm/runtime_root/lib/python3.8/lib-dynload/mmap.so"

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

TEST_CASE("Check function codegen hashing", "[storage]")
{
    // Use two functions we know exist
    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "x2");

    // Load the existing wasm
    std::string pathA = faabric::util::getFunctionFile(msgA);
    std::string pathB = faabric::util::getFunctionFile(msgB);
    std::vector<uint8_t> wasmA = faabric::util::readFileToBytes(pathA);
    std::vector<uint8_t> wasmB = faabric::util::readFileToBytes(pathB);

    msgA.set_inputdata(faabric::util::bytesToString(wasmA));
    msgB.set_inputdata(faabric::util::bytesToString(wasmB));

    // Override the storage directories
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string origFuncDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;
    conf.functionDir = "/tmp/func";
    conf.objectFileDir = "/tmp/obj";

    // Make sure directories are empty to start with
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    // Check wasm files are created and hashes are different
    std::string wasmFileA = "/tmp/func/demo/echo/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/x2/function.wasm";
    std::string objectFileA = "/tmp/obj/demo/echo/function.wasm.o";
    std::string objectFileB = "/tmp/obj/demo/x2/function.wasm.o";
    std::string hashFileA = objectFileA + ".md5";
    std::string hashFileB = objectFileB + ".md5";

    // Check files don't yet exist
    REQUIRE(!boost::filesystem::exists(hashFileA));
    REQUIRE(!boost::filesystem::exists(hashFileB));

    // Upload both
    storage::FileLoader& loader = storage::getFileLoader();
    loader.uploadFunction(msgA);
    loader.uploadFunction(msgB);

    // Check hashes now exist
    REQUIRE(boost::filesystem::exists(hashFileA));
    REQUIRE(boost::filesystem::exists(hashFileB));

    // Read in hashes
    std::vector<uint8_t> actualHashA =
      faabric::util::readFileToBytes(hashFileA);
    std::vector<uint8_t> actualHashB =
      faabric::util::readFileToBytes(hashFileB);

    // Check they're not empty
    REQUIRE(!actualHashA.empty());
    REQUIRE(!actualHashB.empty());

    // Check they're different
    REQUIRE(actualHashA != actualHashB);

    // Write some dummy content to an object file (to check it doesn't get
    // overwritten)
    std::vector<uint8_t> objABefore =
      faabric::util::readFileToBytes(objectFileA);
    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(objectFileA, dummyBytes);

    // Upload the same function and check the object file isn't overwritten
    loader.uploadFunction(msgA);
    std::vector<uint8_t> dummyBytesAfter =
      faabric::util::readFileToBytes(objectFileA);
    REQUIRE(dummyBytesAfter == dummyBytes);

    // Now write the dummy bytes to the hash file and rerun the upload
    faabric::util::writeBytesToFile(hashFileA, dummyBytes);
    loader.uploadFunction(msgA);

    // Check the object file is updated
    std::vector<uint8_t> objAAfter =
      faabric::util::readFileToBytes(objectFileA);
    REQUIRE(objAAfter == objABefore);

    // Check the hash is updated
    std::vector<uint8_t> hashAAfter = faabric::util::readFileToBytes(hashFileA);
    REQUIRE(hashAAfter == actualHashA);

    // Clean up
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    // Reset the config
    conf.functionDir = origFuncDir;
    conf.objectFileDir = origObjDir;
}

TEST_CASE("Check shared object codegen hashing", "[storage]")
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string origObjDir = conf.objectFileDir;
    conf.objectFileDir = "/tmp/obj";

    std::string inputPath = SHARED_OBJ;
    std::string objFile =
      std::string("/tmp/obj") + std::string(SHARED_OBJ) + ".o";
    std::string hashFile = objFile + ".md5";

    // Clean directory beforehand
    boost::filesystem::remove_all(conf.objectFileDir);

    // Run the codegen
    storage::FileLoader& loader = storage::getFileLoader();
    loader.codegenForSharedObject(inputPath);

    // Check both files exist
    REQUIRE(boost::filesystem::exists(objFile));
    REQUIRE(boost::filesystem::exists(hashFile));

    // Read object file and hash to check results later
    std::vector<uint8_t> objBefore = faabric::util::readFileToBytes(objFile);
    std::vector<uint8_t> hashBefore = faabric::util::readFileToBytes(hashFile);

    // Add dummy content to object file
    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(objFile, dummyBytes);

    // Rerun codegen and check not overwritten
    loader.codegenForSharedObject(inputPath);
    std::vector<uint8_t> dummyBytesAfter =
      faabric::util::readFileToBytes(objFile);
    REQUIRE(dummyBytesAfter == dummyBytes);

    // Now write the dummy bytes to the hash file and rerun the upload
    faabric::util::writeBytesToFile(hashFile, dummyBytes);
    loader.codegenForSharedObject(inputPath);

    // Check the object file is updated
    std::vector<uint8_t> objAfter = faabric::util::readFileToBytes(objFile);
    REQUIRE(objAfter == objBefore);

    // Check the hash is updated
    std::vector<uint8_t> hashAfter = faabric::util::readFileToBytes(hashFile);
    REQUIRE(hashAfter == hashBefore);

    // Tidy up
    boost::filesystem::remove_all(conf.objectFileDir);
    conf.objectFileDir = origObjDir;
}
}
