#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <storage/FileserverFileLoader.h>
#include <storage/LocalFileLoader.h>
#include <upload/UploadServer.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <stdlib.h>

using namespace storage;

namespace tests {
TEST_CASE("Test fileserver file loader pulling files over HTTP", "[storage]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    std::string origFunctionStorage = conf.functionStorage;
    std::string origUrl = conf.fileserverUrl;

    conf.functionStorage = "fileserver";
    conf.fileserverUrl = "http://localhost:8002";

    // Load the expected bytes from the function file
    faabric::Message msg = faabric::util::messageFactory("demo", "echo");
    std::string expectedPath = conf::getFunctionFile(msg);
    std::vector<uint8_t> expectedBytes =
      faabric::util::readFileToBytes(expectedPath);

    // Run upload fileserver in the background
    edge::UploadServer server;
    std::thread t([&server] {
        printf("Test upload server listening\n");
        server.listen(UPLOAD_PORT);
    });

    // Wait for the server to start
    usleep(1000 * 1000);

    // Use the fileserver loader to load the file both with and without
    // filesystem caching
    bool useFsCache;
    SECTION("With cache") { useFsCache = true; }
    SECTION("Without cache") { useFsCache = false; }

    storage::FileserverFileLoader loader(useFsCache);
    std::vector<uint8_t> actualBytes = loader.loadFunctionWasm(msg);

    REQUIRE(actualBytes.size() > 1000);
    REQUIRE(actualBytes.size() == expectedBytes.size());
    REQUIRE(expectedBytes == actualBytes);

    server.stop();

    if (t.joinable()) {
        t.join();
    }

    conf.functionStorage = origFunctionStorage;
    conf.fileserverUrl = origUrl;
}

TEST_CASE("Test flushing function files deletes them", "[storage]")
{
    cleanSystem();

    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string origStorage = conf.functionStorage;
    std::string origUrl = conf.fileserverUrl;
    std::string origFunctionDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;

    conf.functionStorage = "fileserver";
    conf.fileserverUrl = "dummy_url";
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

    // Check files still exist
    REQUIRE(!boost::filesystem::exists(funcFile));
    REQUIRE(!boost::filesystem::exists(funcFile));

    // Reset config
    conf.functionStorage = origStorage;
    conf.fileserverUrl = origUrl;
    conf.functionDir = origFunctionDir;
    conf.objectFileDir = origObjDir;
}
}
