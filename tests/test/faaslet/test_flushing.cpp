#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>
#include <faabric/util/testing.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>
#include <storage/SharedFiles.h>
#include <wavm/IRModuleCache.h>
#include <wavm/WAVMWasmModule.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <filesystem>

namespace tests {

class FlushingTestFixture
  : public FunctionLoaderTestFixture
  , public FunctionCallClientServerFixture
  , public SchedulerFixture
{
  public:
    FlushingTestFixture()
      : faabricConf(faabric::util::getSystemConfig())
    {
        uploadTestWasm();

        faabric::util::setTestMode(false);
    }

    ~FlushingTestFixture()
    {
        faabric::util::setTestMode(true);
        faabricConf.reset();
    }

  protected:
    faabric::util::SystemConfig& faabricConf;
};

TEST_CASE_METHOD(FlushingTestFixture,
                 "Test flushing clears shared files",
                 "[faaslet]")
{
    std::string fileName = "flush-test.txt";
    std::vector<uint8_t> fileBytes = { 0, 1, 2, 3 };

    // Upload shared file
    loader.uploadSharedFile(fileName, fileBytes);

    // Check that the underlying shared file is in place
    std::filesystem::path sharedPath(faasmConf.sharedFilesDir);
    sharedPath.append(fileName);
    REQUIRE(std::filesystem::exists(sharedPath));

    // Check that the shared file exists
    std::string syncSharedPath = "faasm://" + fileName;
    REQUIRE(storage::SharedFiles::syncSharedFile(syncSharedPath, "") == 0);

    // Flush and check file is gone
    loader.clearLocalCache();

    // Check the shared files dir exists, but not the specific shared file path
    REQUIRE(std::filesystem::exists(faasmConf.sharedFilesDir));
    REQUIRE(!std::filesystem::exists(sharedPath));

    SECTION("Check using FileLoader")
    {
        loader.loadSharedFile(fileName);
        REQUIRE(std::filesystem::exists(sharedPath));
    }

    SECTION("Check using SharedFiles")
    {
        // Check that if we sync it back, we can open the file
        REQUIRE(storage::SharedFiles::syncSharedFile(syncSharedPath, "") == 0);
        std::string realPath =
          storage::SharedFiles::realPathForSharedFile(syncSharedPath);
        REQUIRE(::open(realPath.c_str(), O_RDONLY) > 0);
    }
}

TEST_CASE_METHOD(FlushingTestFixture,
                 "Test flushing clears cached modules",
                 "[faaslet]")
{
    // Note, these have to be executed in a separate thread to fit with the
    // module's isolation expectation
    std::thread tA(
      [this] { wasm::getWAVMModuleCache().getCachedModule(msgA); });

    std::thread tB(
      [this] { wasm::getWAVMModuleCache().getCachedModule(msgB); });

    if (tA.joinable()) {
        tA.join();
    }
    if (tB.joinable()) {
        tB.join();
    }

    wasm::WAVMModuleCache& cache = wasm::getWAVMModuleCache();
    REQUIRE(cache.getTotalCachedModuleCount() == 2);

    faabric::scheduler::getExecutorFactory()->flushHost();
    REQUIRE(cache.getTotalCachedModuleCount() == 0);
}

TEST_CASE_METHOD(FlushingTestFixture,
                 "Test flushing clears IR module cache",
                 "[faaslet]")
{
    // Execute a task
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    faabric::scheduler::ExecutorContext::set(nullptr, req, 0);
    faaslet::Faaslet f(msg);
    f.executeTask(0, 0, req);

    // Check the module has been cached
    wasm::IRModuleCache& cache = wasm::getIRModuleCache();
    REQUIRE(cache.isModuleCached("demo", "echo", ""));

    // Flush and check it's gone
    faabric::scheduler::getExecutorFactory()->flushHost();
    REQUIRE(!cache.isModuleCached("demo", "echo", ""));

    f.shutdown();
}

TEST_CASE_METHOD(FlushingTestFixture,
                 "Test flushing picks up new version of function",
                 "[faaslet]")
{
    faabricConf.boundTimeout = 1000;

    // Confirm functions are different to start with
    REQUIRE(wasmBytesA.size() != wasmBytesB.size());

    // Set up input/ output
    std::string expectedOutputA = "Hello Faasm!";
    std::string inputB = "This should be echoed";

    // Prepare upload messages for the same dummy function with different wasm
    faabric::Message uploadMsgA = faabric::util::messageFactory("demo", "foo");
    uploadMsgA.set_inputdata(wasmBytesA.data(), wasmBytesA.size());
    faabric::Message uploadMsgB = faabric::util::messageFactory("demo", "foo");
    uploadMsgB.set_inputdata(wasmBytesB.data(), wasmBytesB.size());

    // Upload the first version
    loader.uploadFunction(uploadMsgA);

    // Run the codegen
    codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();
    gen.codegenForFunction(uploadMsgA);

    // Set up faaslet to listen for relevant function
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Call the function
    auto invokeReqA = faabric::util::batchExecFactory("demo", "foo", 1);
    auto invokeMsgA = invokeReqA->messages(0);
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    faabric::scheduler::setExecutorFactory(fac);
    plannerCli.callFunctions(invokeReqA);

    // Check the result
    faabric::Message resultA = plannerCli.getMessageResult(invokeMsgA, 5000);
    REQUIRE(resultA.returnvalue() == 0);
    REQUIRE(resultA.outputdata() == expectedOutputA);

    // Wait for the executor to have cleared up
    SLEEP_MS(1000);

    // Flush
    sch.flushLocally();

    // Upload the second version and check wasm is as expected
    auto invokeReqB = faabric::util::batchExecFactory("demo", "foo", 1);
    auto invokeMsgB = invokeReqB->messages(0);
    loader.uploadFunction(uploadMsgB);
    gen.codegenForFunction(uploadMsgB);

    std::vector<uint8_t> wasmAfterUpload = loader.loadFunctionWasm(invokeMsgB);
    REQUIRE(wasmAfterUpload == wasmBytesB);

    // Invoke for the second time
    invokeReqB->mutable_messages(0)->set_inputdata(inputB);
    plannerCli.callFunctions(invokeReqB);

    // Check the output has changed to the second function
    faabric::Message resultB = plannerCli.getMessageResult(invokeMsgB, 1000);
    REQUIRE(resultB.returnvalue() == 0);
    REQUIRE(resultB.outputdata() == inputB);

    m.shutdown();
}
}
