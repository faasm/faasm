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

TEST_CASE_METHOD(FunctionExecTestFixture, "Test mmapping a file", "[wasm]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // File we know to exist
    std::string fileName = "/usr/include/stdio.h";
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1) {
        FAIL("Could not open file");
    }

    struct stat sb
    {};
    fstat(fd, &sb);
    size_t bufferSize = sb.st_size;

    U32 mappedWasmPtr = module.mmapFile(fd, bufferSize);
    U8* hostPtr = &Runtime::memoryRef<U8>(module.defaultMemory, mappedWasmPtr);

    // Get a section of bytes from the start
    int byteLen = 500;
    std::vector<U8> actual(hostPtr, hostPtr + byteLen);

    // Close the file
    close(fd);

    // Read in by other means
    std::vector<uint8_t> full = faabric::util::readFileToBytes(fileName);
    std::vector<uint8_t> expected(full.data(), full.data() + byteLen);

    // Check the bytes match
    REQUIRE(expected == actual);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test memory growth and shrinkage",
                 "[wasm]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // Check we can mmap less than a page and it rounds up
    size_t oldMemSize = module.getMemorySizeBytes();
    uint32_t oldBrk = module.getCurrentBrk();
    uint32_t memOffset = module.mmapMemory(1);
    size_t newMemSize = module.getMemorySizeBytes();
    size_t newBrk = module.getCurrentBrk();

    REQUIRE(oldBrk == oldMemSize);
    REQUIRE(newBrk == newMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newMemSize == oldMemSize + WASM_BYTES_PER_PAGE);

    // Check we can only grow page-aligned
    REQUIRE_THROWS(module.growMemory(1));

    // Check a page-aligned growth
    oldMemSize = module.getMemorySizeBytes();
    oldBrk = module.getCurrentBrk();

    uint32_t growA = 10 * WASM_BYTES_PER_PAGE;
    memOffset = module.growMemory(growA);
    newMemSize = module.getMemorySizeBytes();
    newBrk = module.getCurrentBrk();

    REQUIRE(newBrk == newMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newMemSize == oldMemSize + growA);

    // Check shrinking memory reduces brk but not size
    oldMemSize = module.getMemorySizeBytes();
    oldBrk = module.getCurrentBrk();

    uint32_t shrinkA = 2 * WASM_BYTES_PER_PAGE;
    memOffset = module.shrinkMemory(shrinkA);
    newMemSize = module.getMemorySizeBytes();
    newBrk = module.getCurrentBrk();

    REQUIRE(memOffset == oldBrk);
    REQUIRE(oldBrk == oldMemSize);
    REQUIRE(newBrk == oldMemSize - shrinkA);
    REQUIRE(newMemSize == oldMemSize);

    // Check growing back up reclaims memory
    oldMemSize = module.getMemorySizeBytes();
    oldBrk = module.getCurrentBrk();

    memOffset = module.growMemory(shrinkA);
    newMemSize = module.getMemorySizeBytes();
    newBrk = module.getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(memOffset == oldBrk);
    REQUIRE(newBrk == newMemSize);

    // Check unmapping at the top of memory shrinks down
    uint32_t shrinkB = 5 * WASM_BYTES_PER_PAGE;
    oldMemSize = module.getMemorySizeBytes();
    oldBrk = module.getCurrentBrk();
    uint32_t unmapOffset = oldMemSize - shrinkB;

    module.unmapMemory(unmapOffset, shrinkB);

    newMemSize = module.getMemorySizeBytes();
    newBrk = module.getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(newBrk == oldMemSize - shrinkB);

    // Check unmapping elsewhere cannot reclaim memory
    uint32_t shrinkC = 3 * WASM_BYTES_PER_PAGE;
    oldMemSize = module.getMemorySizeBytes();
    oldBrk = module.getCurrentBrk();
    unmapOffset = oldMemSize - (2 * WASM_BYTES_PER_PAGE) - shrinkB;

    module.unmapMemory(unmapOffset, shrinkC);

    newMemSize = module.getMemorySizeBytes();
    newBrk = module.getCurrentBrk();

    REQUIRE(newMemSize == oldMemSize);
    REQUIRE(newBrk == oldBrk);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test mmap/munmap", "[faaslet]")
{
    checkCallingFunctionGivesBoolOutput("demo", "mmap", true);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test big mmap", "[faaslet]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "mmap_big");
    execFunction(msg);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test allocating over max memory",
                 "[wasm]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    std::shared_ptr<wasm::WasmModule> module = nullptr;

    std::string expectedMessage;
    SECTION("WAVM")
    {
        module = std::make_shared<wasm::WAVMWasmModule>();
        expectedMessage = "Memory growth exceeding max";
    }

    SECTION("WAMR")
    {
        module = std::make_shared<wasm::WAMRWasmModule>();
        expectedMessage = "WAMR grow memory exceeding max";
    }

    module->bindToFunction(call);

    size_t oneKib = 1024L;
    size_t oneMib = 1024L * oneKib;
    size_t oneGib = 1024L * oneMib;

    // We have to hope this works, otherwise we may cause an OOM on the host
    size_t tenGib = 10L * oneGib;
    bool failed = false;
    try {
        module->setMemorySize(tenGib);
    } catch (std::runtime_error& ex) {
        failed = true;
        std::string actualMessage = ex.what();
        REQUIRE(actualMessage == expectedMessage);
    }

    REQUIRE(failed);
}
}
