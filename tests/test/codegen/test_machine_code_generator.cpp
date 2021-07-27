#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <stdlib.h>

using namespace codegen;

namespace tests {

class CodegenTestFixture
{
  public:
    CodegenTestFixture()
      : conf(conf::getFaasmConfig())
      , loader(storage::getFileLoader())
    {
        s3.createBucket(conf.s3Bucket);
    };

    ~CodegenTestFixture()
    {
        s3.deleteBucket(conf.s3Bucket);
        loader.clearLocalCache();
        conf.reset();
    };

  protected:
    conf::FaasmConfig& conf;
    storage::FileLoader& loader;
    storage::S3Wrapper s3;
};

TEST_CASE_METHOD(CodegenTestFixture,
                 "Check function codegen hashing",
                 "[codegen]")
{
    std::string objectFileA;
    std::string objectFileB;

    bool isCodegenRepeatable = true;
    SECTION("WAVM codegen")
    {
        conf.wasmVm = "wavm";
        objectFileA = "/tmp/obj/demo/echo/function.wasm.o";
        objectFileB = "/tmp/obj/demo/x2/function.wasm.o";
    }

    SECTION("WAMR codegen")
    {
        conf.wasmVm = "wamr";
        objectFileA = "/tmp/obj/demo/echo/function.aot";
        objectFileB = "/tmp/obj/demo/x2/function.aot";

        // It seems that WAMR codegen doesn't produce the same results every
        // time, but they are the same length. Perhaps a timestamp is included.
        isCodegenRepeatable = false;
    }

    codegen::MachineCodeGenerator gen(loader);

    // Use two functions we know exist
    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "x2");

    // Load the existing wasm
    std::string pathA = loader.getFunctionFile(msgA);
    std::string pathB = loader.getFunctionFile(msgB);
    std::vector<uint8_t> wasmA = faabric::util::readFileToBytes(pathA);
    std::vector<uint8_t> wasmB = faabric::util::readFileToBytes(pathB);

    msgA.set_inputdata(faabric::util::bytesToString(wasmA));
    msgB.set_inputdata(faabric::util::bytesToString(wasmB));

    // Override the storage directories
    conf.functionDir = "/tmp/func";
    conf.objectFileDir = "/tmp/obj";

    std::string wasmFileA = "/tmp/func/demo/echo/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/x2/function.wasm";
    std::string hashFileA = objectFileA + HASH_EXT;
    std::string hashFileB = objectFileB + HASH_EXT;

    // Make sure directories are empty to start with
    boost::filesystem::remove_all(conf.functionDir);
    boost::filesystem::remove_all(conf.objectFileDir);

    // Check files don't yet exist
    REQUIRE(!boost::filesystem::exists(hashFileA));
    REQUIRE(!boost::filesystem::exists(hashFileB));

    // Do codegen for both
    gen.codegenForFunction(msgA);
    gen.codegenForFunction(msgB);

    // Check keys exist in S3
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 6);

    // Read in hashes
    std::vector<uint8_t> actualHashA =
      faabric::util::readFileToBytes(hashFileA);
    std::vector<uint8_t> actualHashB =
      faabric::util::readFileToBytes(hashFileB);

    // Check hashes now exist locally
    REQUIRE(boost::filesystem::exists(hashFileA));
    REQUIRE(boost::filesystem::exists(hashFileB));

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

    // Rerun the codegen and check the object file doesn't change
    gen.codegenForFunction(msgA);

    std::vector<uint8_t> dummyBytesAfter = loader.loadFunctionObjectFile(msgA);
    REQUIRE(dummyBytesAfter == dummyBytes);

    // Flush again
    loader.clearLocalCache();

    // Now change the hash file and check the object file *is* overwritten
    loader.uploadFunctionObjectHash(msgA, dummyBytes);
    gen.codegenForFunction(msgA);

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

TEST_CASE_METHOD(CodegenTestFixture,
                 "Check shared object codegen hashing",
                 "[codegen]")
{
    bool localCache;
    SECTION("With local cache") { localCache = true; }

    SECTION("Without local cache") { localCache = false; }

    storage::FileLoader loader(localCache);
    codegen::MachineCodeGenerator gen(loader);

    conf.objectFileDir = "/tmp/obj";

    std::string localSharedObjFile =
      "/usr/local/faasm/runtime_root/lib/python3.8/lib-dynload/mmap.so";
    std::vector<uint8_t> originalObjBytes =
      faabric::util::readFileToBytes(localSharedObjFile);

    std::string inputPath = localSharedObjFile;
    std::string objFile =
      std::string("/tmp/obj") + std::string(localSharedObjFile) + ".o";
    std::string hashFile = objFile + HASH_EXT;

    loader.uploadSharedObjectObjectFile(inputPath, originalObjBytes);

    // Flush anything cached locally
    loader.clearLocalCache();

    // Run the codegen
    gen.codegenForSharedObject(inputPath);

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
    gen.codegenForSharedObject(inputPath);
    std::vector<uint8_t> objAfterA =
      loader.loadSharedObjectObjectFile(inputPath);
    REQUIRE(objAfterA == dummyBytes);

    // Now write the dummy bytes to the hash file and rerun the upload
    loader.uploadSharedObjectObjectHash(inputPath, dummyBytes);
    loader.clearLocalCache();
    gen.codegenForSharedObject(inputPath);

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
}
