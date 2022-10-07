#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/ocalls.h>
#include <wasm/WasmCommon.h>

#include <string>
#include <vector>

namespace wasm {

// Define the module map and its mutex
std::unordered_map<uint32_t, std::shared_ptr<EnclaveWasmModule>> moduleMap;
std::mutex moduleMapMutex;

// Define the WAMR's heap buffer
static uint8_t wamrHeapBuffer[FAASM_SGX_WAMR_HEAP_SIZE];

bool EnclaveWasmModule::initialiseWAMRGlobally()
{
    // Initialise the WAMR runtime
    RuntimeInitArgs wamrRteArgs;
    memset(&wamrRteArgs, 0x0, sizeof(wamrRteArgs));
    wamrRteArgs.mem_alloc_type = Alloc_With_Pool;
    wamrRteArgs.mem_alloc_option.pool.heap_buf = (void*)wamrHeapBuffer;
    wamrRteArgs.mem_alloc_option.pool.heap_size = sizeof(wamrHeapBuffer);

    // Returns true if success, false otherwise
    return wasm_runtime_full_init(&wamrRteArgs);
}

EnclaveWasmModule::EnclaveWasmModule() {}

EnclaveWasmModule::~EnclaveWasmModule()
{
    wasm_runtime_deinstantiate(moduleInstance);
    wasm_runtime_unload(wasmModule);
}

bool EnclaveWasmModule::loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize)
{
    std::vector<uint8_t> wasmBytes((uint8_t*)wasmOpCodePtr,
                                   (uint8_t*)wasmOpCodePtr + wasmOpCodeSize);

    wasmModule = wasm_runtime_load(wasmBytes.data(),
                                   wasmBytes.size(),
                                   errorBuffer,
                                   FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE);

    if (wasmModule == nullptr) {
        return false;
    }

    moduleInstance =
      wasm_runtime_instantiate(wasmModule,
                               FAASM_SGX_WAMR_INSTANCE_DEFAULT_STACK_SIZE,
                               FAASM_SGX_WAMR_INSTANCE_DEFAULT_HEAP_SIZE,
                               errorBuffer,
                               FAASM_SGX_WAMR_MODULE_ERROR_BUFFER_SIZE);

    return moduleInstance != nullptr;
}

bool EnclaveWasmModule::callFunction(uint32_t argcIn, char** argvIn)
{
    WASMFunctionInstanceCommon* func =
      wasm_runtime_lookup_function(moduleInstance, WASM_ENTRY_FUNC, nullptr);

    prepareArgcArgv(argcIn, argvIn);

    // Set dummy argv to capture return value
    std::vector<uint32_t> argv = { 0 };

    bool success =
      aot_create_exec_env_and_call_function((AOTModuleInstance*)moduleInstance,
                                            (AOTFunctionInstance*)func,
                                            0x0,
                                            argv.data());

    if (success) {
        SPDLOG_DEBUG_SGX("Success calling WASM function");
    } else {
        std::string errorMessage(
          ((AOTModuleInstance*)moduleInstance)->cur_exception);
        SPDLOG_ERROR_SGX("Caught WASM runtime exception: %s",
                         errorMessage.c_str());
    }

    return success;
}

WASMModuleInstanceCommon* EnclaveWasmModule::getModuleInstance()
{
    return moduleInstance;
}

void EnclaveWasmModule::prepareArgcArgv(uint32_t argcIn, char** argvIn)
{
    argc = argcIn;
    argv.resize(argc);
    argvBufferSize = 0;

    for (int i = 0; i < argc; i++) {
        // Add one to account for newline
        argvBufferSize += strlen(argvIn[i]) + 1;
        argv.at(i) = std::string(argvIn[i]);
    }
}

uint32_t EnclaveWasmModule::getArgc()
{
    return argc;
}

std::vector<std::string> EnclaveWasmModule::getArgv()
{
    return argv;
}

size_t EnclaveWasmModule::getArgvBufferSize()
{
    return argvBufferSize;
}

std::shared_ptr<EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv)
{
    // Acquiring a lock every time may be too conservative
    std::unique_lock<std::mutex> lock(moduleMapMutex);
    for (auto& it : moduleMap) {
        if (it.second->getModuleInstance() == execEnv->module_inst) {
            return it.second;
        }
    }

    // Returning a null ptr means that we haven't been able to link the
    // execution environment to any of the registered modules. This is a fatal
    // error, but we expect the caller to handle it, as throwing exceptions
    // is not supported.
    SPDLOG_ERROR_SGX("Can not find any registered module corresponding to the "
                     "supplied execution environment, this is a fatal error");

    return nullptr;
}

uint32_t EnclaveWasmModule::getCurrentBrk() const
{
    return currentBrk;
}

uint32_t EnclaveWasmModule::shrinkMemory(size_t nBytes)
{
    if (!isWasmPageAligned((int32_t) nBytes)) {
        SPDLOG_ERROR_SGX("Shrink size not page aligned %li", nBytes);
        throw std::runtime_error("New break not page aligned");
    }

    uint32_t oldBrk = currentBrk;

    if (nBytes > oldBrk) {
        SPDLOG_ERROR_SGX(
          "Shrinking by more than current brk (%li > %i)", nBytes, oldBrk);
        throw std::runtime_error("Shrinking by more than current brk");
    }

    // Note - we don't actually free the memory, we just change the brk
    uint32_t newBrk = oldBrk - nBytes;

    SPDLOG_DEBUG_SGX("MEM - shrinking memory %i -> %i", oldBrk, newBrk);
    currentBrk = newBrk;

    return oldBrk;
}

uint32_t EnclaveWasmModule::growMemory(size_t nBytes)
{
    if (nBytes == 0) {
        return currentBrk;
    }

    /*
    uint32_t oldBytes = getMemorySizeBytes();
    uint32_t oldBrk = currentBrk.load(std::memory_order_acquire);
    uint32_t newBrk = oldBrk + nBytes;

    if (!isWasmPageAligned(newBrk)) {
        SPDLOG_ERROR_SGX("Growing memory by %li is not wasm page aligned"
                         " (current brk: %i, new brk: %i)",
                         nBytes,
                         oldBrk,
                         newBrk);
        throw std::runtime_error("Non-wasm-page-aligned memory growth");
    }

    size_t newBytes = oldBytes + nBytes;
    uint32_t oldPages = getNumberOfWasmPagesForBytes(oldBytes);
    uint32_t newPages = getNumberOfWasmPagesForBytes(newBytes);
    size_t maxPages = getMaxMemoryPages();

    if (newBytes > UINT32_MAX || newPages > maxPages) {
        SPDLOG_ERROR("Growing memory would exceed max of {} pages (current {}, "
                     "requested {})",
                     maxPages,
                     oldPages,
                     newPages);
        throw std::runtime_error("Memory growth exceeding max");
    }

    // If we can reclaim old memory, just bump the break
    if (newBrk <= oldBytes) {
        SPDLOG_TRACE(
          "MEM - Growing memory using already provisioned {} + {} <= {}",
          oldBrk,
          nBytes,
          oldBytes);

        currentBrk.store(newBrk, std::memory_order_release);

        // Make sure permissions on memory are open
        size_t newTop = faabric::util::getRequiredHostPages(currentBrk);
        size_t newStart = faabric::util::getRequiredHostPagesRoundDown(oldBrk);
        newTop *= faabric::util::HOST_PAGE_SIZE;
        newStart *= faabric::util::HOST_PAGE_SIZE;
        SPDLOG_TRACE("Reclaiming memory {}-{}", newStart, newTop);

        uint8_t* memBase = getMemoryBase();
        faabric::util::claimVirtualMemory(
          { memBase + newStart, memBase + newTop });

        return oldBrk;
    }

    uint32_t pageChange = newPages - oldPages;
    bool success = doGrowMemory(pageChange);
    if (!success) {
        throw std::runtime_error("Failed to grow memory");
    }

    SPDLOG_TRACE("Growing memory from {} to {} pages (max {})",
                 oldPages,
                 newPages,
                 maxPages);

    size_t newMemorySize = getMemorySizeBytes();
    currentBrk.store(newMemorySize, std::memory_order_release);

    if (newMemorySize != newBytes) {
        SPDLOG_ERROR(
          "Expected new brk ({}) to be old memory plus new bytes ({})",
          newMemorySize,
          newBytes);
        throw std::runtime_error("Memory growth discrepancy");
    }
    */

    return 0;
    // return oldBrk;
}
}
