#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <storage/FileLoader.h>

#include <filesystem>
#include <stdlib.h>

using namespace codegen;

namespace tests {

class CodegenTestFixture : public FunctionLoaderTestFixture
{
  public:
    CodegenTestFixture()
    {
        // Note that we deliberately switch off test mode here so that we can
        // clear the local file loader cache
        faabric::util::setTestMode(false);
    };

    ~CodegenTestFixture()
    {
        loader.clearLocalCache();
        faabric::util::setTestMode(true);
    };
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
    REQUIRE(std::filesystem::exists(objFile));
    std::vector<uint8_t> objBytes = faabric::util::readFileToBytes(objFile);
    REQUIRE(!objBytes.empty());

    // Check expected keys in S3
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == 3);

    // Clear the local cache to remove local copies
    loader.clearLocalCache();
    REQUIRE(!std::filesystem::exists(objFile));

    // Load object data from file loader and check it's the same
    std::vector<uint8_t> loadedBytes = loader.loadFunctionObjectFile(msg);
    REQUIRE(loadedBytes.size() == objBytes.size());
    REQUIRE(loadedBytes == objBytes);
}

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test machine code is synced even if codegen is skipped",
                 "[codegen]")
{
    // Upload message and run codegen
    loader.uploadFunction(msgA);
    gen.codegenForFunction(msgA);

    // Ensure machine code exists
    std::string objFile = loader.getFunctionObjectFile(msgA);
    std::string hashFile = objFile + HASH_EXT;
    REQUIRE(std::filesystem::exists(objFile));
    REQUIRE(std::filesystem::exists(hashFile));

    // Delete machine code locally
    loader.clearLocalCache();
    REQUIRE(!std::filesystem::exists(objFile));
    REQUIRE(!std::filesystem::exists(hashFile));

    // Run codegen again, will skip, and ensure object is synced regardless
    gen.codegenForFunction(msgA);
    REQUIRE(std::filesystem::exists(objFile));
    REQUIRE(std::filesystem::exists(hashFile));
}

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test function codegen hashing",
                 "[codegen]")
{
    std::string objectFileA;
    std::string objectFileB;

    SECTION("WAVM codegen")
    {
        faasmConf.wasmVm = "wavm";
        objectFileA = "/tmp/obj/demo/hello/function.wasm.o";
        objectFileB = "/tmp/obj/demo/echo/function.wasm.o";
    }

    SECTION("WAMR codegen")
    {
        faasmConf.wasmVm = "wamr";
        objectFileA = "/tmp/obj/demo/hello/function.aot";
        objectFileB = "/tmp/obj/demo/echo/function.aot";
    }

    SECTION("SGX codegen")
    {
        faasmConf.wasmVm = "sgx";
        objectFileA = "/tmp/obj/demo/hello/function.aot.sgx";
        objectFileB = "/tmp/obj/demo/echo/function.aot.sgx";
    }

    codegen::MachineCodeGenerator gen(loader);

    // Upload the functions
    loader.uploadFunction(msgA);
    loader.uploadFunction(msgB);

    std::string wasmFileA = "/tmp/func/demo/hello/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/echo/function.wasm";
    std::string hashFileA = objectFileA + HASH_EXT;
    std::string hashFileB = objectFileB + HASH_EXT;

    // Make sure directories are empty to start with
    loader.clearLocalCache();

    // Check files don't yet exist
    REQUIRE(!std::filesystem::exists(hashFileA));
    REQUIRE(!std::filesystem::exists(hashFileB));

    // Do codegen for both
    gen.codegenForFunction(msgA);
    gen.codegenForFunction(msgB);

    // Check keys exist in S3
    REQUIRE(s3.listKeys(faasmConf.s3Bucket).size() == 6);

    // Check hashes now exist locally
    REQUIRE(std::filesystem::exists(hashFileA));
    REQUIRE(std::filesystem::exists(hashFileB));

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
    REQUIRE(std::filesystem::exists(objectFileA));
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

    REQUIRE(objAAfter == objABefore);

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

    // Check the locally cached path matches the expected one
    REQUIRE(loader.getSharedObjectObjectFile(localSharedObjFile) == objFile);

    // Read object file and hash
    std::vector<uint8_t> objBefore =
      loader.loadSharedObjectObjectFile(localSharedObjFile);
    std::vector<uint8_t> hashBefore =
      loader.loadSharedObjectObjectHash(localSharedObjFile);

    REQUIRE(!objBefore.empty());
    REQUIRE(!hashBefore.empty());

    // Check files exist locally
    REQUIRE(std::filesystem::exists(objFile));
    REQUIRE(std::filesystem::exists(hashFile));

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

    // Clear the local file, re-run the codegen, and check that the object
    // file and hash are synced back
    loader.clearLocalCache();
    REQUIRE(!std::filesystem::exists(objFile));
    REQUIRE(!std::filesystem::exists(hashFile));
    gen.codegenForSharedObject(localSharedObjFile);
    REQUIRE(std::filesystem::exists(objFile));
    REQUIRE(std::filesystem::exists(hashFile));
}

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test WAMR and WAMR + SGX codegen for same func in sequence",
                 "[codegen][wamr][sgx]")
{
    std::string objectFile;
    std::string objectFileSgx;

    // Rename messages for more clarity
    faabric::Message msg = msgA;
    faabric::Message msgSgx = msgA;

    objectFile = "/tmp/obj/demo/hello/function.aot";
    objectFileSgx = "/tmp/obj/demo/hello/function.aot.sgx";

    codegen::MachineCodeGenerator gen(loader);

    // Running codegen on same function, so only need to upload function once
    loader.uploadFunction(msg);

    std::string hashFile = objectFile + HASH_EXT;
    std::string hashFileSgx = objectFileSgx + HASH_EXT;

    // Make sure directories are empty to start with
    loader.clearLocalCache();

    // Check files don't yet exist
    REQUIRE(!std::filesystem::exists(hashFile));
    REQUIRE(!std::filesystem::exists(hashFileSgx));

    // Do codegen for both in different orders
    SECTION("SGX first")
    {
        faasmConf.wasmVm = "sgx";
        gen.codegenForFunction(msgSgx);
        REQUIRE(std::filesystem::exists(hashFileSgx));
        REQUIRE(!std::filesystem::exists(hashFile));

        faasmConf.wasmVm = "wamr";
        gen.codegenForFunction(msg);
        REQUIRE(std::filesystem::exists(hashFile));
    }

    SECTION("SGX second")
    {
        faasmConf.wasmVm = "wamr";
        gen.codegenForFunction(msg);
        REQUIRE(std::filesystem::exists(hashFile));
        REQUIRE(!std::filesystem::exists(hashFileSgx));

        faasmConf.wasmVm = "sgx";
        gen.codegenForFunction(msgSgx);
        REQUIRE(std::filesystem::exists(hashFileSgx));
    }

    // Check hashes in S3
    const std::string preffix = "/tmp/obj/";
    const std::vector<std::string> bucketKeys = s3.listKeys(faasmConf.s3Bucket);
    REQUIRE(std::find(bucketKeys.begin(),
                      bucketKeys.end(),
                      objectFile.substr(preffix.length())) != bucketKeys.end());
    REQUIRE(std::find(bucketKeys.begin(),
                      bucketKeys.end(),
                      objectFileSgx.substr(preffix.length())) !=
            bucketKeys.end());
    REQUIRE(std::find(bucketKeys.begin(),
                      bucketKeys.end(),
                      hashFile.substr(preffix.length())) != bucketKeys.end());
    REQUIRE(std::find(bucketKeys.begin(),
                      bucketKeys.end(),
                      hashFileSgx.substr(preffix.length())) !=
            bucketKeys.end());
}
}
