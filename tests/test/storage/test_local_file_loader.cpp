#include "faabric/util/bytes.h"
#include "faabric/util/func.h"
#include <boost/filesystem/operations.hpp>
#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>
#include <faabric/util/files.h>
#include <stdlib.h>
#include <storage/LocalFileLoader.h>

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

TEST_CASE("Check codegen hashing", "[storage]")
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

    // Upload both
    storage::FileLoader& loader = storage::getFileLoader();
    loader.uploadFunction(msgA);
    loader.uploadFunction(msgB);

    // Check wasm files are created and hashes are different
    std::string wasmFileA = "/tmp/func/demo/echo/function.wasm";
    std::string wasmFileB = "/tmp/func/demo/x2/function.wasm";
    std::string objectFileA = "/tmp/obj/demo/echo/function.wasm.o";
    std::string objectFileB = "/tmp/obj/demo/x2/function.wasm.o";
    std::string hashFileA = objectFileA + ".md5";
    std::string hashFileB = objectFileB + ".md5";

    REQUIRE(boost::filesystem::exists(hashFileA));
    REQUIRE(boost::filesystem::exists(hashFileB));

    // Read in hashes
    std::vector<uint8_t> actualHashA = faabric::util::readFileToBytes(hashFileA);
    std::vector<uint8_t> actualHashB = faabric::util::readFileToBytes(hashFileB);

    REQUIRE(!actualHashA.empty());
    REQUIRE(!actualHashB.empty());

    // Reset the config
    conf.functionDir = origFuncDir;
    conf.objectFileDir = origObjDir;
}
}
