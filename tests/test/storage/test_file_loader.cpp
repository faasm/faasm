#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>
#include <upload/UploadServer.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <stdlib.h>

using namespace storage;

namespace tests {

class FileLoaderTestFixture : public FunctionLoaderTestFixture
{
  public:
    FileLoaderTestFixture()
    {
        // Note that we deliberately switch off test mode here so that we can
        // clear the local file loader cache
        faabric::util::setTestMode(false);
    };

    ~FileLoaderTestFixture() { faabric::util::setTestMode(true); };
};

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test file loader pulling function files from S3",
                 "[storage]")
{
    // Use the fileserver loader to load the file both with and without
    // filesystem caching
    bool useFsCache;
    SECTION("With cache") { useFsCache = true; }

    SECTION("Without cache") { useFsCache = false; }

    storage::FileLoader loader(useFsCache);
    loader.clearLocalCache();

    codegen::MachineCodeGenerator gen(loader);

    std::string cachedWasmFile = loader.getFunctionFile(msgB);
    std::string cachedObjFile = loader.getFunctionObjectFile(msgB);
    std::string cachedObjectHash = loader.getHashFilePath(cachedObjFile);

    // Ensure nothing in S3 to start with
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).empty());

    // Upload the function and machine code
    loader.uploadFunction(msgB);
    gen.codegenForFunction(msgB);
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == 3);
    REQUIRE(boost::filesystem::exists(cachedWasmFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjectHash) == useFsCache);

    // Clear everything locally
    loader.clearLocalCache();
    REQUIRE(!boost::filesystem::exists(cachedWasmFile));
    REQUIRE(!boost::filesystem::exists(cachedObjFile));
    REQUIRE(!boost::filesystem::exists(cachedObjectHash));

    // Load the function
    std::vector<uint8_t> actualWasmBytes = loader.loadFunctionWasm(msgB);
    REQUIRE(actualWasmBytes.size() > 1000);
    REQUIRE(actualWasmBytes.size() == wasmBytesB.size());
    REQUIRE(actualWasmBytes == wasmBytesB);

    // Load the machine code
    std::vector<uint8_t> actualObjectBytes =
      loader.loadFunctionObjectFile(msgB);
    REQUIRE(actualObjectBytes.size() == objBytesB.size());
    REQUIRE(actualObjectBytes == objBytesB);

    // Check downloaded files are cached and non-download files aren't
    REQUIRE(boost::filesystem::exists(cachedWasmFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjFile) == useFsCache);
    REQUIRE(!boost::filesystem::exists(cachedObjectHash));
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test clearing local file loader cache",
                 "[storage]")
{
    std::string funcFile = faasmConf.functionDir + "/function.wasm";
    std::string objFile = faasmConf.objectFileDir + "/function.obj";

    // Clean directories
    boost::filesystem::remove_all(faasmConf.functionDir);
    boost::filesystem::remove_all(faasmConf.objectFileDir);

    boost::filesystem::create_directories(faasmConf.functionDir);
    boost::filesystem::create_directories(faasmConf.objectFileDir);

    // Write some junk to a couple of files
    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(funcFile, bytes);
    faabric::util::writeBytesToFile(objFile, bytes);

    REQUIRE(boost::filesystem::exists(funcFile));
    REQUIRE(boost::filesystem::exists(funcFile));

    // Now flush
    loader.clearLocalCache();

    // Check files don't exist
    REQUIRE(!boost::filesystem::exists(funcFile));
    REQUIRE(!boost::filesystem::exists(funcFile));
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test uploading and loading shared files",
                 "[storage]")
{
    // Ensure nothing in S3 to start with
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).empty());

    // Check we can try loading the file and it throws an exception
    std::string relativePath = "test/local_file_loader.txt";
    bool exThrown = false;
    try {
        loader.loadSharedFile(relativePath);
    } catch (SharedFileNotExistsException& e) {
        std::string expected = fmt::format("{} does not exist", relativePath);
        REQUIRE(e.what() == expected);
        exThrown = true;
    }

    REQUIRE(exThrown);

    // Check we can delete when it doesn't exist
    loader.deleteSharedFile(relativePath);

    // Create the shared file
    std::vector<uint8_t> expected = { 1, 5, 3, 2, 4 };
    storage::FileLoader loader;
    loader.uploadSharedFile(relativePath, expected);

    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == 1);

    const std::vector<uint8_t> actual = loader.loadSharedFile(relativePath);
    REQUIRE(actual == expected);

    // Check the expected path
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    boost::filesystem::path fullPath(conf.sharedFilesDir);
    fullPath.append(relativePath);
    REQUIRE(boost::filesystem::exists(fullPath));

    // Check data is as expected
    const std::vector<uint8_t> actualBytes =
      faabric::util::readFileToBytes(fullPath.string());
    REQUIRE(actualBytes == expected);

    // Check it's cleared away when local cache is cleared
    loader.clearLocalCache();
    REQUIRE(!boost::filesystem::exists(fullPath));

    // Check we can still load from S3
    const std::vector<uint8_t> actualAfter =
      loader.loadSharedFile(relativePath);
    REQUIRE(actualAfter == expected);

    // Delete the file
    loader.deleteSharedFile(relativePath);

    // Check it no longer exists
    REQUIRE_THROWS_AS(loader.loadSharedFile(relativePath),
                      SharedFileNotExistsException);
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test uploading and loading python files",
                 "[storage]")
{
    std::vector<uint8_t> contents = { 0, 1, 2, 3 };

    faabric::Message msg;
    msg.set_pythonuser("foo");
    msg.set_pythonfunction("bar");
    msg.set_inputdata(contents.data(), contents.size());

    storage::FileLoader loader;
    loader.uploadPythonFunction(msg);

    std::string relativePath = "pyfuncs/foo/bar/function.py";
    std::string expectedRuntimePath =
      faasmConf.sharedFilesDir + "/" + relativePath;

    REQUIRE(boost::filesystem::exists(expectedRuntimePath));

    loader.clearLocalCache();
    REQUIRE(!boost::filesystem::exists(expectedRuntimePath));

    std::vector<uint8_t> actualBytes = loader.loadSharedFile(relativePath);
    REQUIRE(actualBytes == contents);
    REQUIRE(boost::filesystem::exists(expectedRuntimePath));
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test badly formed python upload messages",
                 "[storage]")
{
    std::vector<uint8_t> contents = { 0, 1, 2, 3 };

    faabric::Message msg;

    SECTION("No function set") { msg.set_pythonuser("foo"); }
    SECTION("No user set") { msg.set_pythonfunction("bar"); }

    msg.set_inputdata(contents.data(), contents.size());

    storage::FileLoader loader;
    REQUIRE_THROWS(loader.uploadPythonFunction(msg));
}
}
