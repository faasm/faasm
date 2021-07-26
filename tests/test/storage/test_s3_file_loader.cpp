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
    std::vector<uint8_t> expectedBytes =
      faabric::util::readFileToBytes(expectedPath);
    msg.set_inputdata(expectedBytes.data(), expectedBytes.size());

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
    std::vector<uint8_t> actualBytes = loader.loadFunctionWasm(msg);
    REQUIRE(actualBytes.size() > 1000);
    REQUIRE(actualBytes.size() == expectedBytes.size());
    REQUIRE(expectedBytes == actualBytes);

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
    loader.flushFunctionFiles();

    // Check files don't exist
    REQUIRE(!boost::filesystem::exists(funcFile));
    REQUIRE(!boost::filesystem::exists(funcFile));
}
}
