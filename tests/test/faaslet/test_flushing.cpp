#include <catch2/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <storage/FileLoader.h>
#include <wavm/IRModuleCache.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {

TEST_CASE("Test flushing clears shared files", "[flush]")
{
    cleanSystem();

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
    faabric::Message msg = faabric::util::messageFactory("demo", "echo");
    faaslet::Faaslet f(msg);
    f.flush();
    REQUIRE(!boost::filesystem::exists(sharedPath));
}

TEST_CASE("Test flushing clears cached modules", "[flush]")
{
    cleanSystem();

    const faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    const faabric::Message msgB =
      faabric::util::messageFactory("demo", "dummy");

    // Note, these have to be executed in a separate thread to fit with the
    // module's isolation expectation
    std::thread tA(
      [&msgA] { wasm::getWAVMModuleCache().getCachedModule(msgA); });

    std::thread tB(
      [&msgB] { wasm::getWAVMModuleCache().getCachedModule(msgB); });

    if (tA.joinable()) {
        tA.join();
    }
    if (tB.joinable()) {
        tB.join();
    }

    wasm::WAVMModuleCache& cache = wasm::getWAVMModuleCache();
    REQUIRE(cache.getTotalCachedModuleCount() == 2);

    faaslet::Faaslet f(msgA);
    f.flush();
    REQUIRE(cache.getTotalCachedModuleCount() == 0);
}

TEST_CASE("Test flushing clears IR module cache", "[flush]")
{
    cleanSystem();

    // Execute a task
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    faaslet::Faaslet f(msg);
    f.executeTask(0, 0, req);

    // Check the module has been cached
    wasm::IRModuleCache& cache = wasm::getIRModuleCache();
    REQUIRE(cache.isModuleCached("demo", "echo", ""));

    // Flush and check it's gone
    f.flush();
    REQUIRE(!cache.isModuleCached("demo", "echo", ""));
}

TEST_CASE("Test flushing picks up new version of function", "[flush]")
{
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    int origBoundTimeout = conf.boundTimeout;
    conf.boundTimeout = 1000;

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
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Call the function
    faabric::Message invokeMsgA = faabric::util::messageFactory("demo", "foo");
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(invokeMsgA);

    // Check the result
    faabric::Message resultA = sch.getFunctionResult(invokeMsgA.id(), 1000);
    REQUIRE(resultA.returnvalue() == 0);
    REQUIRE(resultA.outputdata() == expectedOutputA);

    // Wait for the executor to have cleared up
    usleep(1000 * 1000);

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

    conf.boundTimeout = origBoundTimeout;
    conf.functionDir = origFunctionDir;
    conf.objectFileDir = origObjDir;

    m.shutdown();
}
}
