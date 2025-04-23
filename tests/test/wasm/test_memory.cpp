#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace WAVM;

namespace tests {

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test mmapping a file",
                 "[wasm]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    std::shared_ptr<wasm::WasmModule> module = nullptr;

    /*
    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
        module = std::make_shared<wasm::WAVMWasmModule>();
    }
    */

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
        module = std::make_shared<wasm::WAMRWasmModule>();
    }

    module->bindToFunction(call);

    // File we know to exist
    std::string fileName = "/usr/lib/x86_64-linux-gnu/libLLVM-17.so.1";
    int hostFd = open(fileName.c_str(), O_RDONLY);
    if (hostFd == -1) {
        FAIL("Could not open file");
    }

    struct stat statBuf
    {};
    fstat(hostFd, &statBuf);
    size_t bufferSize = statBuf.st_size;

    int32_t mappedWasmPtr = module->mmapFile(hostFd, bufferSize);
    uint8_t* hostPtr = module->wasmPointerToNative(mappedWasmPtr);

    // Get a section of bytes from the start
    int byteLen = 500 * 10;
    std::vector<uint8_t> actual(hostPtr, hostPtr + byteLen);

    // Close the file
    close(hostFd);

    // Read in by other means
    std::vector<uint8_t> full = faabric::util::readFileToBytes(fileName);
    std::vector<uint8_t> expected(full.data(), full.data() + byteLen);

    // Check the bytes match
    auto equal = expected == actual;
    REQUIRE(equal);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test memory growth and shrinkage",
                 "[wasm]")
{
    std::shared_ptr<wasm::WasmModule> module = nullptr;
    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    // Test different WASM VMs
    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
        module = std::make_shared<wasm::WAVMWasmModule>();
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
        module = std::make_shared<wasm::WAMRWasmModule>();
    }

    module->bindToFunction(call);

    // Check we can mmap less than a page and it rounds up
    size_t oldMemSize = module->getMemorySizeBytes();
    uint32_t oldBrk = module->getCurrentBrk();
    uint32_t memOffset = module->mmapMemory(1);
    size_t newMemSize = module->getMemorySizeBytes();
    size_t newBrk = module->getCurrentBrk();

    REQUIRE(oldBrk == oldMemSize);
    REQUIRE(newBrk == newMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newMemSize == oldMemSize + WASM_BYTES_PER_PAGE);

    // Check we can only grow page-aligned
    REQUIRE_THROWS(module->growMemory(1));

    // Check a page-aligned growth
    oldMemSize = module->getMemorySizeBytes();
    oldBrk = module->getCurrentBrk();

    uint32_t growA = 10 * WASM_BYTES_PER_PAGE;
    memOffset = module->growMemory(growA);
    newMemSize = module->getMemorySizeBytes();
    newBrk = module->getCurrentBrk();

    REQUIRE(newBrk == newMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newMemSize == oldMemSize + growA);

    // Check shrinking memory reduces brk but not size
    oldMemSize = module->getMemorySizeBytes();
    oldBrk = module->getCurrentBrk();

    uint32_t shrinkA = 2 * WASM_BYTES_PER_PAGE;
    memOffset = module->shrinkMemory(shrinkA);
    newMemSize = module->getMemorySizeBytes();
    newBrk = module->getCurrentBrk();

    REQUIRE(memOffset == oldBrk);
    REQUIRE(oldBrk == oldMemSize);
    REQUIRE(newBrk == oldMemSize - shrinkA);
    REQUIRE(newMemSize == oldMemSize);

    // Check growing back up reclaims memory
    oldMemSize = module->getMemorySizeBytes();
    oldBrk = module->getCurrentBrk();

    memOffset = module->growMemory(shrinkA);
    newMemSize = module->getMemorySizeBytes();
    newBrk = module->getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newBrk == newMemSize);

    // Check unmapping at the top of memory shrinks down
    uint32_t shrinkB = 5 * WASM_BYTES_PER_PAGE;
    oldMemSize = module->getMemorySizeBytes();
    oldBrk = module->getCurrentBrk();
    uint32_t unmapOffset = oldMemSize - shrinkB;

    module->unmapMemory(unmapOffset, shrinkB);

    newMemSize = module->getMemorySizeBytes();
    newBrk = module->getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(newBrk == oldMemSize - shrinkB);

    // Check unmapping elsewhere cannot reclaim memory
    uint32_t shrinkC = 3 * WASM_BYTES_PER_PAGE;
    oldMemSize = module->getMemorySizeBytes();
    oldBrk = module->getCurrentBrk();
    unmapOffset = oldMemSize - (2 * WASM_BYTES_PER_PAGE) - shrinkB;

    module->unmapMemory(unmapOffset, shrinkC);

    newMemSize = module->getMemorySizeBytes();
    newBrk = module->getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(newBrk == oldBrk);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test mmap/munmap",
                 "[wasm]")
{
    auto req = setUpContext("demo", "mmap");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifdef FAASM_SGX_HARDWARE_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    REQUIRE(executeWithPoolGetBooleanResult(req));
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test big mmap", "[wasm]")
{
    auto req = setUpContext("demo", "mmap_big");
    int timeoutMs = 1000;

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifdef FAASM_SGX_HARDWARE_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
        // This test does a lot of new memory allocations, which patricularly
        // stress SGX's EDMM feature. Therefore we can only run it in HW
        // mode
        timeoutMs = 20 * 1000;
    }
#endif

    executeWithPool(req, timeoutMs);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test allocating over max memory",
                 "[wasm]")
{
    auto req = setUpContext("demo", "echo");
    faabric::Message& call = req->mutable_messages()->at(0);

    std::shared_ptr<wasm::WasmModule> module = nullptr;

    std::string expectedMessage = "Memory growth exceeding max";
    SECTION("WAVM")
    {
        module = std::make_shared<wasm::WAVMWasmModule>();
    }

    SECTION("WAMR")
    {
        module = std::make_shared<wasm::WAMRWasmModule>();
    }

    module->bindToFunction(call);

    size_t oneKib = 1024L;
    size_t oneMib = 1024L * oneKib;
    size_t oneGib = 1024L * oneMib;

    // We have to hope this works, otherwise we may cause an OOM on the host
    size_t fiveGib = 5L * oneGib;
    bool failed = false;
    try {
        module->setMemorySize(fiveGib);
    } catch (std::runtime_error& ex) {
        failed = true;
        std::string actualMessage = ex.what();
        REQUIRE(actualMessage == expectedMessage);
    }

    REQUIRE(failed);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test memcpy", "[memory]")
{
    auto req = setUpContext("demo", "memcpy");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifdef FAASM_SGX_HARDWARE_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test memmove",
                 "[memory]")
{
    auto req = setUpContext("demo", "memmove");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifdef FAASM_SGX_HARDWARE_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    executeWithPool(req);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture, "Test calloc", "[memory]")
{
    auto req = setUpContext("demo", "calloc");

    SECTION("WAVM")
    {
        faasmConf.wasmVm = "wavm";
    }

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

#ifdef FAASM_SGX_HARDWARE_MODE
    SECTION("SGX")
    {
        faasmConf.wasmVm = "sgx";
    }
#endif

    executeWithPool(req);
}
}
