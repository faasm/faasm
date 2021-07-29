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

class FileLoaderTestFixture : public S3TestFixture
{
  public:
    FileLoaderTestFixture()
    {
        // Load the wasm for an existing function
        faabric::Message msg = faabric::util::messageFactory("demo", "echo");
        wasmFilePath = conf.functionDir + "/demo/echo/function.wasm";
        objFilePath = conf.objectFileDir + "/demo/echo/function.wasm.o";

        if (!boost::filesystem::exists(wasmFilePath)) {
            SPDLOG_ERROR("Did not find function wasm file at {}", wasmFilePath);
            throw std::runtime_error("Did not find expected wasm file");
        }

        if (!boost::filesystem::exists(objFilePath)) {
            SPDLOG_ERROR("Did not find function object file at {}",
                         objFilePath);
            throw std::runtime_error("Did not find expected object file");
        }

        wasmBytes = faabric::util::readFileToBytes(wasmFilePath);
        objBytes = faabric::util::readFileToBytes(objFilePath);

        // Note that we deliberately switch off test mode here so that we can
        // clear the local file loader cache
        faabric::util::setTestMode(false);

        // Dummy directories for functions and object files
        conf.functionDir = "/tmp/func";
        conf.objectFileDir = "/tmp/obj";
    };

    ~FileLoaderTestFixture() { faabric::util::setTestMode(true); };

  protected:
    std::string wasmFilePath;
    std::string objFilePath;
    std::vector<uint8_t> wasmBytes;
    std::vector<uint8_t> objBytes;
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

    faabric::Message msg = faabric::util::messageFactory("demo", "echo");

    // Set the input data to the message for the upload
    msg.set_inputdata(wasmBytes.data(), wasmBytes.size());

    std::string cachedWasmFile = loader.getFunctionFile(msg);
    std::string cachedObjFile = loader.getFunctionObjectFile(msg);
    std::string cachedObjectHash = loader.getHashFilePath(cachedObjFile);

    // Check nothing in S3 to start with
    REQUIRE(s3.listKeys(conf.s3Bucket).empty());

    // Upload the function and machine code
    loader.uploadFunction(msg);
    gen.codegenForFunction(msg);
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 3);
    REQUIRE(boost::filesystem::exists(cachedWasmFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjectHash) == useFsCache);

    // Clear everything locally
    loader.clearLocalCache();
    REQUIRE(!boost::filesystem::exists(cachedWasmFile));
    REQUIRE(!boost::filesystem::exists(cachedObjFile));
    REQUIRE(!boost::filesystem::exists(cachedObjectHash));

    // Load the function
    std::vector<uint8_t> actualWasmBytes = loader.loadFunctionWasm(msg);
    REQUIRE(actualWasmBytes.size() > 1000);
    REQUIRE(actualWasmBytes.size() == wasmBytes.size());
    REQUIRE(actualWasmBytes == wasmBytes);

    // Load the machine code
    std::vector<uint8_t> actualObjectBytes = loader.loadFunctionObjectFile(msg);
    REQUIRE(actualObjectBytes.size() == objBytes.size());
    REQUIRE(actualObjectBytes == objBytes);

    // Check downloaded files are cached and non-download files aren't
    REQUIRE(boost::filesystem::exists(cachedWasmFile) == useFsCache);
    REQUIRE(boost::filesystem::exists(cachedObjFile) == useFsCache);
    REQUIRE(!boost::filesystem::exists(cachedObjectHash));
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test clearing local file loader cache",
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

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test uploading and loading shared files",
                 "[storage]")
{
    std::string relativePath = "test/local_file_loader.txt";
    std::vector<uint8_t> expected = { 1, 5, 3, 2, 4 };

    storage::FileLoader loader;
    loader.uploadSharedFile(relativePath, expected);

    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 1);

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
}

TEST_CASE_METHOD(FileLoaderTestFixture,
                 "Test uploading and loading python files",
                 "[storage]")
{
    std::vector<uint8_t> contents = { 0, 1, 2, 3 };

    std::string pythonUser = "foo";
    std::string pythonFunction = "bar";

    std::string relativePath = "pyfuncs/foo/bar/function.py";
    std::string expectedSharedFilePath = "faasm://" + relativePath;
    std::string expectedRuntimePath = conf.sharedFilesDir + "/" + relativePath;

    faabric::Message msg;
    msg.set_pythonuser(pythonUser);
    msg.set_pythonfunction(pythonFunction);
    msg.set_inputdata(contents.data(), contents.size());

    storage::FileLoader loader;
    loader.uploadPythonFunction(msg);

    REQUIRE(boost::filesystem::exists(expectedRuntimePath));
    REQUIRE(loader.getPythonFunctionSharedFilePath(msg) ==
            expectedSharedFilePath);

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
