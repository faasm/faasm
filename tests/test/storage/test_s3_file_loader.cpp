#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <storage/FileLoader.h>
#include <upload/UploadServer.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <stdlib.h>

using namespace storage;

namespace tests {

class S3FilesTestFixture
{
  public:
    S3FilesTestFixture()
      : conf(conf::getFaasmConfig())
    {
        s3.createBucket(conf.s3Bucket);
    };

    ~S3FilesTestFixture()
    {
        s3.deleteBucket(conf.s3Bucket);
        conf.reset();
    };

  protected:
    conf::FaasmConfig& conf;
    storage::S3Wrapper s3;
};

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Test file loader pulling files from S3",
                 "[storage]")
{
    // Load the expected bytes from the function file
    faabric::Message msg = faabric::util::messageFactory("demo", "echo");
    std::string expectedPath = conf::getFunctionFile(msg);
    std::vector<uint8_t> wasmBytes =
      faabric::util::readFileToBytes(expectedPath);

    // Get the expected object data
    std::string objPath = conf::getFunctionObjectFile(msg);
    std::vector<uint8_t> objectBytes = faabric::util::readFileToBytes(objPath);

    // Set the input data to the message for the upload
    msg.set_inputdata(wasmBytes.data(), wasmBytes.size());

    std::string cachedWasmFile = conf::getFunctionFile(msg);
    std::string cachedObjFile = conf::getFunctionObjectFile(msg);
    std::string cachedObjectHash = conf::getHashFilePath(cachedObjFile);

    // Delete the cached hash to make sure we regenerate the machine code
    boost::filesystem::remove(cachedObjectHash);

    // Check nothing in S3 to start with
    REQUIRE(s3.listKeys(conf.s3Bucket).empty());

    // Use the fileserver loader to load the file both with and without
    // filesystem caching
    bool useFsCache;
    SECTION("With cache") { useFsCache = true; }

    SECTION("Without cache") { useFsCache = false; }

    storage::FileLoader loader(useFsCache);

    // Upload the function and make sure it's written to S3 along with the
    // machine code and the machine code's hash
    loader.uploadFunction(msg);
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 3);

    // Load the function
    std::vector<uint8_t> actualWasmBytes = loader.loadFunctionWasm(msg);
    REQUIRE(actualWasmBytes.size() > 1000);
    REQUIRE(actualWasmBytes.size() == wasmBytes.size());
    REQUIRE(wasmBytes == actualWasmBytes);

    // Load the machine code
    std::vector<uint8_t> actualObjectBytes = loader.loadFunctionObjectFile(msg);
    REQUIRE(actualObjectBytes == objectBytes);
    REQUIRE(actualObjectBytes.size() == objectBytes.size());
    REQUIRE(objectBytes == actualObjectBytes);

    // Check it's cached locally if necessary
    if (useFsCache) {
        REQUIRE(boost::filesystem::exists(cachedWasmFile));
    } else {
        REQUIRE(!boost::filesystem::exists(cachedWasmFile));
    }
}

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Test flushing function files deletes them locally",
                 "[storage]")
{
    std::string origFunctionDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;

    conf.functionDir = "/tmp/faasm/funcs";
    conf.objectFileDir = "/tmp/faasm/objs";

    std::string funcFile = conf.functionDir + "/function.wasm";
    std::string objFile = conf.objectFileDir + "/function.obj";

    // Clean directories
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    boost::filesystem::create_directories(conf.functionDir);
    boost::filesystem::create_directories(conf.objectFileDir);

    // Write some junk to a couple of files
    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(funcFile, bytes);
    faabric::util::writeBytesToFile(objFile, bytes);

    REQUIRE(boost::filesystem::exists(funcFile));
    REQUIRE(boost::filesystem::exists(funcFile));

    // Now flush
    storage::FileLoader& loader = storage::getFileLoader();
    loader.clearLocalCache();

    // Check files don't exist
    REQUIRE(!boost::filesystem::exists(funcFile));
    REQUIRE(!boost::filesystem::exists(funcFile));
}

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Check uploading and loading shared files",
                 "[storage]")
{
    std::string relativePath = "test/local_file_loader.txt";
    std::vector<uint8_t> expected = { 1, 5, 3, 2, 4 };

    bool localCache = false;
    SECTION("With local cache") { localCache = true; }

    SECTION("Without local cache") { localCache = false; }

    storage::FileLoader loader(localCache);
    loader.uploadSharedFile(relativePath, expected);

    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 1);

    const std::vector<uint8_t> actual = loader.loadSharedFile(relativePath);
    REQUIRE(actual == expected);

    // Check it's written where we expect it to be too
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    boost::filesystem::path fullPath(conf.sharedFilesStorageDir);
    fullPath.append(relativePath);

    if (localCache) {
        const std::vector<uint8_t> actualBytes =
          faabric::util::readFileToBytes(fullPath.string());
        REQUIRE(actualBytes == expected);
    } else {
        REQUIRE(!boost::filesystem::exists(fullPath));
    }
}

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Check function codegen hashing",
                 "[storage]")
{
    // Use two functions we know exist
    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "x2");

    // Load the existing wasm
    std::string pathA = conf::getFunctionFile(msgA);
    std::string pathB = conf::getFunctionFile(msgB);
    std::vector<uint8_t> wasmA = faabric::util::readFileToBytes(pathA);
    std::vector<uint8_t> wasmB = faabric::util::readFileToBytes(pathB);

    msgA.set_inputdata(faabric::util::bytesToString(wasmA));
    msgB.set_inputdata(faabric::util::bytesToString(wasmB));

    // Override the storage directories
    std::string origFuncDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;
    conf.functionDir = "/tmp/func";
    conf.objectFileDir = "/tmp/obj";

    // Test for different wasm VMs
    std::string origWasmVm = conf.wasmVm;
    std::string objectFileA;
    std::string objectFileB;

    std::string wasmFileA = "/tmp/func/demo/echo/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/x2/function.wasm";
    std::string hashFileA = objectFileA + HASH_EXT;
    std::string hashFileB = objectFileB + HASH_EXT;

    bool isCodegenRepeatable = true;
    bool localCache = false;
    SECTION("WAVM codegen")
    {
        conf.wasmVm = "wavm";
        objectFileA = "/tmp/obj/demo/echo/function.wasm.o";
        objectFileB = "/tmp/obj/demo/x2/function.wasm.o";

        SECTION("WAVM with local cache") { localCache = true; }

        SECTION("WAVM without local cache") { localCache = false; }
    }

    SECTION("WAMR codegen")
    {
        conf.wasmVm = "wamr";
        objectFileA = "/tmp/obj/demo/echo/function.aot";
        objectFileB = "/tmp/obj/demo/x2/function.aot";

        // It seems that WAMR codegen doesn't produce the same results every
        // time, but they are the same length. Perhaps a timestamp is included.
        isCodegenRepeatable = false;

        SECTION("WAMR with local cache") { localCache = true; }

        SECTION("WAMR without local cache") { localCache = false; }
    }

    storage::FileLoader loader(localCache);

    // Make sure directories are empty to start with
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    // Check files don't yet exist
    REQUIRE(!boost::filesystem::exists(hashFileA));
    REQUIRE(!boost::filesystem::exists(hashFileB));

    // Upload both
    loader.uploadFunction(msgA);
    loader.uploadFunction(msgB);

    // Check keys exist in S3
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 6);

    // Read in hashes
    std::vector<uint8_t> actualHashA =
      faabric::util::readFileToBytes(hashFileA);
    std::vector<uint8_t> actualHashB =
      faabric::util::readFileToBytes(hashFileB);

    // Check hashes now exist locally if necessary
    REQUIRE(boost::filesystem::exists(hashFileA) == localCache);
    REQUIRE(boost::filesystem::exists(hashFileB) == localCache);

    // Check they're not empty
    REQUIRE(!actualHashA.empty());
    REQUIRE(!actualHashB.empty());

    // Check they're different
    REQUIRE(actualHashA != actualHashB);

    // Load the object file before, then flush
    std::vector<uint8_t> objABefore =
      faabric::util::readFileToBytes(objectFileA);

    loader.clearLocalCache();

    // Now write some dummy content directly to the object file (to check it
    // doesn't get overwritten)
    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    loader.uploadFunctionObjectFile(msgA, dummyBytes);

    // Upload the same function and check the object file isn't regenerated
    loader.uploadFunction(msgA);

    std::vector<uint8_t> dummyBytesAfter = loader.loadFunctionObjectFile(msgA);
    REQUIRE(dummyBytesAfter == dummyBytes);

    // Flush again
    loader.clearLocalCache();

    // Now change the hash file and check the object file *is* overwritten
    loader.uploadFunctionObjectHash(msgA, dummyBytes);
    loader.uploadFunction(msgA);

    std::vector<uint8_t> objAAfter = loader.loadFunctionObjectFile(msgA);

    if (isCodegenRepeatable) {
        REQUIRE(objAAfter == objABefore);
    } else {
        // Just check the length of the machine code if codegen not repeatable
        REQUIRE(objAAfter.size() == objABefore.size());
    }

    // Check the hash is updated
    std::vector<uint8_t> hashAAfter = loader.loadFunctionObjectHash(msgA);
    REQUIRE(hashAAfter == actualHashA);

    // Clean up
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);
}

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Check shared object codegen hashing",
                 "[storage]")
{
    std::string origObjDir = conf.objectFileDir;
    conf.objectFileDir = "/tmp/obj";

    std::string localSharedObjFile =
      "/usr/local/faasm/runtime_root/lib/python3.8/lib-dynload/mmap.so";
    std::vector<uint8_t> originalObjBytes =
      faabric::util::readFileToBytes(localSharedObjFile);

    std::string inputPath = localSharedObjFile;
    std::string objFile =
      std::string("/tmp/obj") + std::string(localSharedObjFile) + ".o";
    std::string hashFile = objFile + HASH_EXT;

    bool localCache;
    SECTION("With local cache") { localCache = true; }

    SECTION("Without local cache") { localCache = false; }

    // Upload the file first
    storage::FileLoader loader(localCache);
    loader.uploadSharedObjectObjectFile(inputPath, originalObjBytes);

    // Flush anything cached locally
    loader.clearLocalCache();

    // Run the codegen
    loader.codegenForSharedObject(inputPath);

    // Read object file and hash
    std::vector<uint8_t> objBefore =
      loader.loadSharedObjectObjectFile(inputPath);
    std::vector<uint8_t> hashBefore =
      loader.loadSharedObjectObjectHash(inputPath);

    REQUIRE(!objBefore.empty());
    REQUIRE(!hashBefore.empty());

    // Check files exist locally if caching
    REQUIRE(boost::filesystem::exists(objFile) == localCache);
    REQUIRE(boost::filesystem::exists(hashFile) == localCache);

    // Remove local cache and write dummy object data
    loader.clearLocalCache();
    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    loader.uploadSharedObjectObjectFile(inputPath, dummyBytes);

    // Rerun codegen and check dummy data not overwritten (i.e. codegen skipped)
    loader.codegenForSharedObject(inputPath);
    std::vector<uint8_t> objAfterA =
      loader.loadSharedObjectObjectFile(inputPath);
    REQUIRE(objAfterA == dummyBytes);

    // Now write the dummy bytes to the hash file and rerun the upload
    loader.uploadSharedObjectObjectHash(inputPath, dummyBytes);
    loader.clearLocalCache();
    loader.codegenForSharedObject(inputPath);

    // Check the object file is updated
    std::vector<uint8_t> objAfterB =
      loader.loadSharedObjectObjectFile(inputPath);
    REQUIRE(objAfterB.size() == objBefore.size());
    REQUIRE(objAfterB == objBefore);

    // Check the hash is updated
    std::vector<uint8_t> hashAfter =
      loader.loadSharedObjectObjectHash(inputPath);
    REQUIRE(hashAfter == hashBefore);

    // Tidy up
    boost::filesystem::remove_all(conf.objectFileDir);
}

TEST_CASE_METHOD(S3FilesTestFixture,
                 "Test flushing function files clears local directories",
                 "[storage]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string origFunctionDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;

    conf.functionDir = "/tmp/faasm/funcs";
    conf.objectFileDir = "/tmp/faasm/objs";

    std::string funcFile = conf.functionDir + "/function.wasm";
    std::string objFile = conf.objectFileDir + "/function.obj";

    // Clean directories
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    boost::filesystem::create_directories(conf.functionDir);
    boost::filesystem::create_directories(conf.objectFileDir);

    // Write some junk to a couple of files
    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(funcFile, bytes);
    faabric::util::writeBytesToFile(objFile, bytes);

    REQUIRE(boost::filesystem::exists(funcFile));
    REQUIRE(boost::filesystem::exists(objFile));

    // Now flush
    storage::FileLoader loader;
    loader.clearLocalCache();

    // Check files no longer exist
    REQUIRE(!boost::filesystem::exists(funcFile));
    REQUIRE(!boost::filesystem::exists(objFile));
}
}
