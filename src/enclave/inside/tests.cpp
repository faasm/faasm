#include <enclave/error.h>
#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>

#include <memory>

// TODO: move elsewhere
#define REQUIRE(cond)                                                          \
    if (!(cond)) {                                                             \
        SPDLOG_ERROR_SGX("Error testing assertion!");                          \
        return FAASM_SGX_INTERNAL_TEST_ERROR;                                  \
    }

#define REQUIRE_THROWS(expr)                                                   \
    try {                                                                      \
        expr;                                                                  \
        SPDLOG_ERROR_SGX("Expression did not throw an exception!");            \
        return FAASM_SGX_INTERNAL_TEST_ERROR;                                  \
    } catch (const std::runtime_error& e) {                                    \
        SPDLOG_DEBUG_SGX("Caught exception: %s", e.what());                    \
    }

namespace tests {

faasm_sgx_status_t testSbrk(std::shared_ptr<wasm::EnclaveWasmModule> module)
{
    size_t initialSize = module->getMemorySizeBytes();
    REQUIRE(module->getCurrentBrk() == initialSize);

    uint32_t growA = 5 * WASM_BYTES_PER_PAGE;
    uint32_t growB = 20 * WASM_BYTES_PER_PAGE;

    module->growMemory(growA);
    size_t sizeA = module->getMemorySizeBytes();
    REQUIRE(sizeA > initialSize);
    REQUIRE(sizeA == initialSize + growA);
    REQUIRE(module->getCurrentBrk() == sizeA);

    module->growMemory(growB);
    size_t sizeB = module->getMemorySizeBytes();
    REQUIRE(sizeB > initialSize + growA);
    REQUIRE(sizeB == initialSize + growA + growB);
    REQUIRE(module->getCurrentBrk() == sizeB);

    // Lastly, run the function to verify that the heap has not been
    // corrupted
    char** argvIn = (char*[]){ "function.wasm" };
    REQUIRE(module->callFunction(1, argvIn) == 0);

    return FAASM_SGX_SUCCESS;
}

faasm_sgx_status_t testMemoryGrowthShrinkage(
  std::shared_ptr<wasm::EnclaveWasmModule> module)
{
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

    return FAASM_SGX_SUCCESS;
}
}
