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

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

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

    SECTION("WAVM codegen")
    {
        conf.wasmVm = "wavm";
        objectFileA = "/tmp/obj/demo/hello/function.wasm.o";
        objectFileB = "/tmp/obj/demo/echo/function.wasm.o";
    }

    SECTION("WAMR codegen")
    {
        conf.wasmVm = "wamr";

        SECTION("Non-SGX")
        {
            objectFileA = "/tmp/obj/demo/hello/function.aot";
            objectFileB = "/tmp/obj/demo/echo/function.aot";
        }

        SECTION("SGX")
        {
            objectFileA = "/tmp/obj/demo/hello/function.aot.sgx";
            objectFileB = "/tmp/obj/demo/echo/function.aot.sgx";
            msgA.set_issgx(true);
            msgB.set_issgx(true);
        }
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

TEST_CASE_METHOD(CodegenTestFixture,
                 "Test WAMR and WAMR + SGX codegen for same func in sequence",
                 "[codegen]")
{
    std::string objectFile;
    std::string objectFileSgx;
    conf.wasmVm = "wamr";

    // Rename messages for more clarity
    faabric::Message msg = msgA;
    faabric::Message msgSgx = msgA;

    msg.set_issgx(false);
    msgSgx.set_issgx(true);
    objectFile = "/tmp/obj/demo/hello/function.aot.sgx";
    objectFileSgx = "/tmp/obj/demo/hello/function.aot";

    codegen::MachineCodeGenerator gen(loader);

    // Running codegen on same function, so only need to upload function once
    loader.uploadFunction(msg);

    std::string hashFile = objectFile + HASH_EXT;
    std::string hashFileSgx = objectFileSgx + HASH_EXT;

    // Make sure directories are empty to start with
    loader.clearLocalCache();

    // Check files don't yet exist
    REQUIRE(!boost::filesystem::exists(hashFile));
    REQUIRE(!boost::filesystem::exists(hashFileSgx));

    // Do codegen for both in different orders
    SECTION("SGX first")
    {
        gen.codegenForFunction(msgSgx);
        gen.codegenForFunction(msg);
    }

    SECTION("SGX second")
    {
        gen.codegenForFunction(msg);
        gen.codegenForFunction(msgSgx);
    }

    // Check hashes now exist locally
    REQUIRE(boost::filesystem::exists(hashFile));
    REQUIRE(boost::filesystem::exists(hashFileSgx));

    // Check hashes in S3
    const std::string preffix = "/tmp/obj/";
    const std::vector<std::string> bucketKeys = s3.listKeys(conf.s3Bucket);
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
