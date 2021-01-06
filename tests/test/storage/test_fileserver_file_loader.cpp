#include "faabric/util/bytes.h"
#include "faabric/util/func.h"

#include "utils.h"

#include <boost/filesystem/operations.hpp>
#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>
#include <faabric/util/files.h>
#include <stdlib.h>
#include <storage/LocalFileLoader.h>

using namespace storage;

namespace tests {
TEST_CASE("Test flushing function files deletes them", "[storage]") {
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
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
    std::vector<uint8_t> bytes = {0, 1, 2, 3};
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
