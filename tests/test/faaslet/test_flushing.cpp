#include <catch2/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>
#include <faabric/executor/FaabricMain.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faaslet/FaasletPool.h>
#include <module_cache/WasmModuleCache.h>
#include <storage/FileLoader.h>

namespace tests {
TEST_CASE("Test flushing empty faaslet", "[faaslet]")
{
    faaslet::Faaslet f(0);
    f.flush();
}

TEST_CASE("Test flushing worker clears state", "[faaslet]")
{
    // Set up some state
    faabric::state::State& state = faabric::state::getGlobalState();
    state.getKV("demo", "blah", 10);
    state.getKV("other", "foo", 30);

    REQUIRE(state.getKVCount() == 2);

    faaslet::Faaslet f(0);
    f.flush();

    REQUIRE(state.getKVCount() == 0);
}

TEST_CASE("Test flushing worker clears shared files", "[faaslet]")
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    std::string relativePath = "flush-test.txt";
    boost::filesystem::path sharedPath(conf.sharedFilesDir);
    sharedPath.append(relativePath);

    if (boost::filesystem::exists(sharedPath)) {
        boost::filesystem::remove(sharedPath);
    }
    boost::filesystem::create_directories(conf.sharedFilesDir);

    // Enter some data
    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(sharedPath.string(), bytes);
    REQUIRE(boost::filesystem::exists(sharedPath));

    // Flush and check file is gone
    faaslet::Faaslet f(0);
    f.flush();
    REQUIRE(!boost::filesystem::exists(sharedPath));
}

TEST_CASE("Test flushing worker clears zygotes", "[faaslet]")
{
    const faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    const faabric::Message msgB =
      faabric::util::messageFactory("demo", "dummy");

    module_cache::WasmModuleCache& reg = module_cache::getWasmModuleCache();
    reg.getCachedModule(msgA);
    reg.getCachedModule(msgB);

    REQUIRE(reg.getTotalCachedModuleCount() == 2);

    faaslet::Faaslet f(0);
    f.flush();
    REQUIRE(reg.getTotalCachedModuleCount() == 0);
}

TEST_CASE("Test flushing worker clears scheduler", "[faaslet]")
{
    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "dummy");

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(msgA);
    sch.callFunction(msgB);

    REQUIRE(sch.getFunctionInFlightCount(msgA) == 1);
    REQUIRE(sch.getFunctionInFlightCount(msgB) == 1);

    faaslet::Faaslet f(0);
    f.flush();

    REQUIRE(sch.getFunctionInFlightCount(msgA) == 0);
    REQUIRE(sch.getFunctionInFlightCount(msgB) == 0);
}

TEST_CASE("Test flushing worker picks up new version of function")
{
    cleanSystem();

    // Load the wasm for the real functions
    faabric::Message origMsgA = faabric::util::messageFactory("demo", "hello");
    faabric::Message origMsgB = faabric::util::messageFactory("demo", "echo");

    std::string expectedOutputA = "Hello Faasm!";
    std::string inputB = "This should be echoed";

    storage::FileLoader& fileLoader = storage::getFileLoader();
    std::vector<uint8_t> wasmA = fileLoader.loadFunctionWasm(origMsgA);
    std::vector<uint8_t> wasmB = fileLoader.loadFunctionWasm(origMsgB);

    // Prepare the upload messages for the dummy function
    faabric::Message uploadMsgA = faabric::util::messageFactory("demo", "foo");
    uploadMsgA.set_inputdata(wasmA.data(), wasmA.size());
    faabric::Message uploadMsgB = faabric::util::messageFactory("demo", "foo");
    uploadMsgB.set_inputdata(wasmB.data(), wasmB.size());

    // Set up dummy directories
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.functionDir = "/tmp/faasm/funcs";
    conf.objectFileDir = "/tmp/faasm/objs";
    std::string origFunctionDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;

    // Upload the first version
    fileLoader.uploadFunction(uploadMsgA);

    // Set up worker to listen for relevant function
    faaslet::FaasletPool pool(1);
    faaslet::Faaslet w(1);
    REQUIRE(!w.isBound());

    // Call the function
    faabric::Message invokeMsgA = faabric::util::messageFactory("demo", "foo");
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(invokeMsgA);

    // Process the bind and execute messages
    w.processNextMessage();
    w.processNextMessage();
    REQUIRE(w.isBound());

    // Check the result
    faabric::Message resultA = sch.getFunctionResult(invokeMsgA.id(), 1);
    REQUIRE(resultA.returnvalue() == 0);
    REQUIRE(resultA.outputdata() == expectedOutputA);

    // Flush
    faabric::Message flushMessage =
      faabric::util::messageFactory("demo", "foo");
    flushMessage.set_type(faabric::Message_MessageType_FLUSH);
    sch.callFunction(flushMessage);

    // Check the Faaslet is no longer bound
    REQUIRE(!w.isBound());

    // Upload the second version and invoke
    fileLoader.uploadFunction(uploadMsgB);
    faabric::Message invokeMsgB = faabric::util::messageFactory("demo", "foo");
    sch.callFunction(invokeMsgB);

    // Check the output has changed to the second function
    faabric::Message resultB = sch.getFunctionResult(invokeMsgB.id(), 1);
    REQUIRE(resultB.returnvalue() == 0);
    REQUIRE(resultB.outputdata() == inputB);

    // Process the bind and execute messages
    w.processNextMessage();
    w.processNextMessage();
    REQUIRE(w.isBound());

    conf.functionDir = origFunctionDir;
    conf.objectFileDir = origObjDir;
}
}
