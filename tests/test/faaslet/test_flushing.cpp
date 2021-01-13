#include <catch2/catch.hpp>

#include "faabric/util/queue.h"
#include "ir_cache/IRModuleCache.h"
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
TEST_CASE("Test flushing empty faaslet does not break", "[faaslet]")
{
    faaslet::Faaslet f(0);
    REQUIRE_THROWS_AS(f.flush(), faabric::util::ExecutorFinishedException);
}

TEST_CASE("Test flushing faaslet clears shared files", "[faaslet]")
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
    REQUIRE_THROWS_AS(f.flush(), faabric::util::ExecutorFinishedException);
    REQUIRE(!boost::filesystem::exists(sharedPath));
}

TEST_CASE("Test flushing faaslet clears zygotes", "[faaslet]")
{
    const faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    const faabric::Message msgB =
      faabric::util::messageFactory("demo", "dummy");

    module_cache::WasmModuleCache& reg = module_cache::getWasmModuleCache();
    reg.getCachedModule(msgA);
    reg.getCachedModule(msgB);

    REQUIRE(reg.getTotalCachedModuleCount() == 2);

    faaslet::Faaslet f(0);
    REQUIRE_THROWS_AS(f.flush(), faabric::util::ExecutorFinishedException);
    REQUIRE(reg.getTotalCachedModuleCount() == 0);
}

TEST_CASE("Test flushing faaslet clears IR module cache", "[faaslet]")
{
    const faabric::Message msg = faabric::util::messageFactory("demo", "echo");

    faaslet::Faaslet f(0);
    f.bindToFunction(msg);

    wasm::IRModuleCache& cache = wasm::getIRModuleCache();
    REQUIRE(cache.isModuleCached("demo", "echo", ""));

    REQUIRE_THROWS_AS(f.flush(), faabric::util::ExecutorFinishedException);

    REQUIRE(!cache.isModuleCached("demo", "echo", ""));
}

TEST_CASE("Test flushing faaslet picks up new version of function")
{
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    int origBoundTimeout = conf.boundTimeout;
    int origUnboundTimeout = conf.unboundTimeout;
    conf.boundTimeout = 1000;
    conf.unboundTimeout = 1000;

    // Load the wasm for two real functions
    faabric::Message origMsgA = faabric::util::messageFactory("demo", "hello");
    faabric::Message origMsgB = faabric::util::messageFactory("demo", "echo");

    storage::FileLoader& fileLoader = storage::getFileLoader();
    std::vector<uint8_t> wasmA = fileLoader.loadFunctionWasm(origMsgA);
    std::vector<uint8_t> wasmB = fileLoader.loadFunctionWasm(origMsgB);

    // Check they're different
    REQUIRE(wasmA.size() != wasmB.size());

    // Set up input/ output
    std::string expectedOutputA = "Hello Faasm!";
    std::string inputB = "This should be echoed";

    // Prepare upload messages for the same dummy function with different wasm
    faabric::Message uploadMsgA = faabric::util::messageFactory("demo", "foo");
    uploadMsgA.set_inputdata(wasmA.data(), wasmA.size());
    faabric::Message uploadMsgB = faabric::util::messageFactory("demo", "foo");
    uploadMsgB.set_inputdata(wasmB.data(), wasmB.size());

    // Set up dummy directories for storage
    conf.functionDir = "/tmp/faasm/funcs";
    conf.objectFileDir = "/tmp/faasm/objs";
    std::string origFunctionDir = conf.functionDir;
    std::string origObjDir = conf.objectFileDir;

    // Upload the first version
    fileLoader.uploadFunction(uploadMsgA);

    // Set up faaslet to listen for relevant function
    faaslet::FaasletPool pool(1);
    pool.startThreadPool();

    // Call the function
    faabric::Message invokeMsgA = faabric::util::messageFactory("demo", "foo");
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(invokeMsgA);

    // Check the result
    faabric::Message resultA = sch.getFunctionResult(invokeMsgA.id(), 1000);
    REQUIRE(resultA.returnvalue() == 0);
    REQUIRE(resultA.outputdata() == expectedOutputA);

    // Flush
    sch.flushLocally();

    // Upload the second version and check wasm is as expected
    faabric::Message invokeMsgB = faabric::util::messageFactory("demo", "foo");
    fileLoader.uploadFunction(uploadMsgB);
    std::vector<uint8_t> wasmAfterUpload =
      fileLoader.loadFunctionWasm(invokeMsgB);
    REQUIRE(wasmAfterUpload == wasmB);

    // Invoke for the second time
    invokeMsgB.set_inputdata(inputB);
    sch.callFunction(invokeMsgB);

    // Check the output has changed to the second function
    faabric::Message resultB = sch.getFunctionResult(invokeMsgB.id(), 1);
    REQUIRE(resultB.returnvalue() == 0);
    REQUIRE(resultB.outputdata() == inputB);

    pool.shutdown();

    conf.boundTimeout = origBoundTimeout;
    conf.unboundTimeout = origUnboundTimeout;
    conf.functionDir = origFunctionDir;
    conf.objectFileDir = origObjDir;
}
}
