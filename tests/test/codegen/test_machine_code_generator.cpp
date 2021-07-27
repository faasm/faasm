#include <catch2/catch.hpp>

#include "faabric/util/testing.h"
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
      , gen(codegen::getMachineCodeGenerator())
    {
        // Use two functions we know exist
        faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
        faabric::Message msgB = faabric::util::messageFactory("demo", "x2");

        // Load some wasm data for existing functions
        std::string pathA = loader.getFunctionFile(msgA);
        std::string pathB = loader.getFunctionFile(msgB);

        wasmBytesA = faabric::util::readFileToBytes(pathA);
        wasmBytesB = faabric::util::readFileToBytes(pathB);
        sharedObjWasm = faabric::util::readFileToBytes(localSharedObjFile);

        // Note that we deliberately switch off test mode here so that we can
        // clear the local file loaded cache
        faabric::util::setTestMode(false);

        conf.s3Bucket = "faasm-test";
        conf.functionDir = "/tmp/func";
        conf.objectFileDir = "/tmp/obj";

        s3.createBucket(conf.s3Bucket);
    };

    ~CodegenTestFixture()
    {
        loader.clearLocalCache();

        s3.deleteBucket(conf.s3Bucket);

        faabric::util::setTestMode(true);
        conf.reset();
    };

  protected:
    conf::FaasmConfig& conf;
    storage::FileLoader& loader;
    codegen::MachineCodeGenerator& gen;
    storage::S3Wrapper s3;

    std::string localSharedObjFile =
      "/usr/local/faasm/runtime_root/lib/python3.8/lib-dynload/mmap.so";

    std::vector<uint8_t> wasmBytesA;
    std::vector<uint8_t> wasmBytesB;
    std::vector<uint8_t> sharedObjWasm;
};

TEST_CASE_METHOD(CodegenTestFixture, "Test basic codegen", "[codegen]")
{
    // Upload existing function to new user/ function name
    std::string user = "blah";
    std::string function = "foo";
    faabric::Message msg = faabric::util::messageFactory(user, function);
    msg.set_inputdata(wasmBytesA.data(), wasmBytesA.size());

    loader.uploadFunction(msg);

    // Run codegen
    gen.codegenForFunction(msg);

    // Read in the object data
    std::string objFile = loader.getFunctionObjectFile(msg);
    REQUIRE(boost::filesystem::exists(objFile));
    std::vector<uint8_t> objBytes = faabric::util::readFileToBytes(objFile);
    REQUIRE(!objBytes.empty());

    // Check expected keys in S3
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 3);

    // Clear the local cache to remove local copies
    loader.clearLocalCache();
    REQUIRE(!boost::filesystem::exists(objFile));

    // Load object data from file loader and check it's the same
    std::vector<uint8_t> loadedBytes = loader.loadFunctionObjectFile(msg);
    REQUIRE(loadedBytes.size() == objBytes.size());
    REQUIRE(loadedBytes == objBytes);
}

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test function codegen hashing",
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

    msgA.set_inputdata(wasmBytesA.data(), wasmBytesA.size());
    msgB.set_inputdata(wasmBytesB.data(), wasmBytesB.size());

    // Upload the functions
    loader.uploadFunction(msgA);
    loader.uploadFunction(msgB);

    std::string wasmFileA = "/tmp/func/demo/echo/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/x2/function.wasm";
    std::string hashFileA = objectFileA + HASH_EXT;
    std::string hashFileB = objectFileB + HASH_EXT;

    // Make sure directories are empty to start with
    loader.clearLocalCache();

    // Check files don't yet exist
    REQUIRE(!boost::filesystem::exists(hashFileA));
    REQUIRE(!boost::filesystem::exists(hashFileB));

    // Do codegen for both
    gen.codegenForFunction(msgA);
    gen.codegenForFunction(msgB);

    // Check keys exist in S3
    REQUIRE(s3.listKeys(conf.s3Bucket).size() == 6);

    // Check hashes now exist locally
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

    // Load the object file before, then flush
    REQUIRE(boost::filesystem::exists(objectFileA));
    std::vector<uint8_t> objABefore =
      faabric::util::readFileToBytes(objectFileA);

    // Now write some dummy content directly to the object file (to check it
    // doesn't get overwritten)
    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(objectFileA, dummyBytes);

    // Rerun the codegen and check the object file doesn't change
    gen.codegenForFunction(msgA);

    std::vector<uint8_t> dummyBytesAfter =
      faabric::util::readFileToBytes(objectFileA);
    REQUIRE(dummyBytesAfter == dummyBytes);

    // Now change the hash file and check the object file *is* overwritten
    faabric::util::writeBytesToFile(hashFileA, dummyBytes);
    gen.codegenForFunction(msgA);

    std::vector<uint8_t> objAAfter =
      faabric::util::readFileToBytes(objectFileA);

    if (isCodegenRepeatable) {
        REQUIRE(objAAfter == objABefore);
    } else {
        // Just check the length of the machine code if codegen not repeatable
        REQUIRE(objAAfter.size() == objABefore.size());
    }

    // Check the hash is updated
    std::vector<uint8_t> hashAAfter = faabric::util::readFileToBytes(hashFileA);
    REQUIRE(hashAAfter == actualHashA);
}

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test shared object codegen hashing",
                 "[codegen]")
{
    std::string objFile =
      std::string("/tmp/obj") + std::string(localSharedObjFile) + ".o";
    std::string hashFile = objFile + HASH_EXT;

    loader.uploadSharedObjectObjectFile(localSharedObjFile, sharedObjWasm);

    // Flush anything cached locally
    loader.clearLocalCache();

    // Run the codegen
    gen.codegenForSharedObject(localSharedObjFile);

    // Read object file and hash
    std::vector<uint8_t> objBefore =
      loader.loadSharedObjectObjectFile(localSharedObjFile);
    std::vector<uint8_t> hashBefore =
      loader.loadSharedObjectObjectHash(localSharedObjFile);

    REQUIRE(!objBefore.empty());
    REQUIRE(!hashBefore.empty());

    // Check files exist locally
    REQUIRE(boost::filesystem::exists(objFile));
    REQUIRE(boost::filesystem::exists(hashFile));

    std::vector<uint8_t> dummyBytes = { 0, 1, 2, 3 };
    loader.uploadSharedObjectObjectFile(localSharedObjFile, dummyBytes);

    // Rerun codegen and check dummy data not overwritten (i.e. codegen skipped)
    gen.codegenForSharedObject(localSharedObjFile);
    std::vector<uint8_t> objAfterA =
      loader.loadSharedObjectObjectFile(localSharedObjFile);
    REQUIRE(objAfterA == dummyBytes);

    // Now write the dummy bytes to the hash file and rerun the upload
    loader.uploadSharedObjectObjectHash(localSharedObjFile, dummyBytes);
    gen.codegenForSharedObject(localSharedObjFile);

    // Check the object file is updated
    std::vector<uint8_t> objAfterB =
      loader.loadSharedObjectObjectFile(localSharedObjFile);
    REQUIRE(objAfterB.size() == objBefore.size());
    REQUIRE(objAfterB == objBefore);

    // Check the hash is updated
    std::vector<uint8_t> hashAfter =
      loader.loadSharedObjectObjectHash(localSharedObjFile);
    REQUIRE(hashAfter == hashBefore);
}
}
