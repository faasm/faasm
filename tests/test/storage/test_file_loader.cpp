#include <catch2/catch.hpp>

#include "codegen/MachineCodeGenerator.h"
#include "faabric_utils.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>

#include <conf/FaasmConfig.h>
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
    // Use the fileserver loader to load the file both with and without
    // filesystem caching
    bool useFsCache;
    SECTION("With cache") { useFsCache = true; }

    SECTION("Without cache") { useFsCache = false; }

    storage::FileLoader loader(useFsCache);
    codegen::MachineCodeGenerator gen(loader);

    // Load the expected bytes from the function file
    faabric::Message msg = faabric::util::messageFactory("demo", "echo");
    std::string expectedPath = loader.getFunctionFile(msg);
    std::vector<uint8_t> wasmBytes =
      faabric::util::readFileToBytes(expectedPath);

    // Get the expected object data
    std::string objPath = loader.getFunctionObjectFile(msg);
    std::vector<uint8_t> objectBytes = faabric::util::readFileToBytes(objPath);

    // Set the input data to the message for the upload
    msg.set_inputdata(wasmBytes.data(), wasmBytes.size());

    std::string cachedWasmFile = loader.getFunctionFile(msg);
    std::string cachedObjFile = loader.getFunctionObjectFile(msg);
    std::string cachedObjectHash = loader.getHashFilePath(cachedObjFile);

    // Delete the cached hash to make sure we regenerate the machine code
    boost::filesystem::remove(cachedObjectHash);

    // Check nothing in S3 to start with
    REQUIRE(s3.listKeys(conf.s3Bucket).empty());

    // Upload the function and machine code
    loader.uploadFunction(msg);
    gen.codegenForFunction(msg);
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
                 "Test flushing function files clears local directories",
                 "[storage]")
{
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
