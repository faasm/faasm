#include "wasm.h"

#include <util/files.h>
#include <util/func.h>
#include <util/memory.h>
#include <prof/prof.h>

#include <syscall.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/Runtime/RuntimeData.h>


using namespace WAVM;

namespace wasm {
    static thread_local WasmModule *executingModule;
    static thread_local message::Message *executingCall;
    static thread_local CallChain *executingCallChain;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    message::Message *getExecutingCall() {
        return executingCall;
    }

    CallChain *getExecutingCallChain() {
        return executingCallChain;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() {
        // delete[] cleanMemory;

        defaultMemory = nullptr;
        moduleInstance = nullptr;
        functionInstance = nullptr;

        if (compartment != nullptr) {
            resolver->cleanUp();

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            bool compartmentCleared = Runtime::tryCollectCompartment(std::move(compartment));
            if (!compartmentCleared) {
                logger->debug("Failed GC for compartment");
            } else {
                logger->debug("Successful GC for compartment");
            }
        }
    };

    bool WasmModule::isBound() {
        return _isBound;
    }

    bool WasmModule::isInitialised() {
        return _isInitialised;
    }

    void WasmModule::initialise() {
        if (compartment != nullptr) {
            throw std::runtime_error("Cannot initialise already initialised module");
        }

        // Treat any unhandled exception (e.g. in a thread) as a fatal error.
        Runtime::setUnhandledExceptionHandler([](Runtime::Exception &&exception) {
            Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
        });

        compartment = Runtime::createCompartment();

        // Prepare name resolution
        resolver = new RootResolver(compartment);

        _isInitialised = true;
    }

    void WasmModule::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            throw std::runtime_error("Must initialise module before binding");
        } else if (_isBound) {
            throw std::runtime_error("Cannot bind a module twice");
        }

        // Parse the wasm file to work out imports, function signatures etc.
        const util::TimePoint &wasmParseTs = prof::startTimer();
        this->parseWasm(msg);
        prof::logEndTimer("wasm-parse", wasmParseTs);

        // Linking
        const util::TimePoint &linkTs = prof::startTimer();
        resolver->setUser(msg.user());
        Runtime::LinkResult linkResult = linkModule(module, *resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            throw std::runtime_error("Failed linking module");
        }
        prof::logEndTimer("wasm-link", linkTs);

        // Load the object file
        const util::TimePoint &objTs = prof::startTimer();
        std::vector<uint8_t> objectFileBytes = util::getFunctionObjectBytes(msg);
        prof::logEndTimer("obj-load", objTs);

        // Instantiate the module, i.e. create memory, tables etc.
        const util::TimePoint &instantTs = prof::startTimer();
        Runtime::ModuleRef compiledModule = Runtime::loadPrecompiledModule(module, objectFileBytes);
        moduleInstance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                util::funcToString(msg)
        );
        prof::logEndTimer("wavm-mod", instantTs);

        // Extract the module's exported function
        // Note that an underscore may be added before the function name by the compiler
        functionInstance = asFunctionNullable(
                getInstanceExport(moduleInstance, ENTRYPOINT_FUNC));

        if (!functionInstance) {
            std::string errorMsg = "No exported function \"" + ENTRYPOINT_FUNC + "\"";
            throw std::runtime_error(errorMsg);
        }

        // Keep reference to memory
        this->defaultMemory = Runtime::getDefaultMemory(moduleInstance);

        // Snapshot initial state
        const util::TimePoint &memTs = prof::startTimer();
        this->snapshotMemory();
        prof::logEndTimer("mem-snap", memTs);

        // Record that this module is now bound
        _isBound = true;
        boundUser = msg.user();
        boundFunction = msg.function();
    }

    void WasmModule::snapshotMemory() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->debug("Snapshotting {} pages of memory for restore", CLEAN_MEMORY_PAGES);
        cleanMemory = new uint8_t[CLEAN_MEMORY_SIZE];

        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        std::copy(baseAddr, baseAddr + CLEAN_MEMORY_SIZE, cleanMemory);
    }

    void WasmModule::restoreMemory() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Grow/ shrink memory to its original size
        Uptr currentPages = Runtime::getMemoryNumPages(this->defaultMemory);

        if (currentPages > INITIAL_MEMORY_PAGES) {
            Uptr shrinkSize = currentPages - INITIAL_MEMORY_PAGES;
            logger->debug("Restoring memory and shrinking {} pages", shrinkSize);

            Runtime::shrinkMemory(this->defaultMemory, shrinkSize);

        } else if (INITIAL_MEMORY_PAGES > currentPages) {
            Uptr growSize = INITIAL_MEMORY_PAGES - currentPages;
            logger->debug("Restoring memory and growing {} pages", growSize);

            Runtime::growMemory(this->defaultMemory, growSize);

        } else {
            logger->debug("Restoring memory with {} pages", INITIAL_MEMORY_PAGES);
        }

        // Restore initial memory in clean region
        const util::TimePoint &memRestoreTs = prof::startTimer();
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        std::copy(cleanMemory, cleanMemory + CLEAN_MEMORY_SIZE, baseAddr);
        prof::logEndTimer("mem-restore", memRestoreTs);

        // Reset shared memory variables
        sharedMemKVs.clear();
        sharedMemWasmPtrs.clear();
        sharedMemHostPtrs.clear();

        // Unmap shared memory regions
//        for (auto const &p : sharedMemWasmPtrs) {
//            state::StateKeyValue *kv = sharedMemKVs[p.first];
//            void* hostPtr = sharedMemHostPtrs[p.first];
//            kv->unmapSharedMemory(hostPtr);
//        }
    }

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::Message &msg, CallChain &callChain) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!_isBound) {
            throw std::runtime_error("WorkerThread must be bound before executing function");
        } else if (boundUser != msg.user() || boundFunction != msg.function()) {
            logger->error("Cannot execute {} on module bound to {}/{}",
                          util::funcToString(msg), boundUser, boundFunction);
            throw std::runtime_error("Cannot execute function on module bound to another");
        }

        // Set up shared references
        executingModule = this;
        executingCall = &msg;
        executingCallChain = &callChain;

        // Make the call
        int exitCode = 0;
        std::vector<IR::Value> invokeArgs;
        try {
            // Create the runtime context
            Runtime::Context *context = Runtime::createContext(compartment);

            // Call the function
            invokeFunctionChecked(context, functionInstance, invokeArgs);
        }
        catch (wasm::WasmExitException &e) {
            exitCode = e.exitCode;
        }
        return exitCode;
    }

    std::vector<uint8_t> WasmModule::compile(message::Message &msg) {
        // Parse the wasm file to work out imports, function signatures etc.
        WasmModule tempModule;
        tempModule.parseWasm(msg);

        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(tempModule.module);
        return Runtime::getObjectCode(module);
    }

    void WasmModule::compileToObjectFile(message::Message &msg) {
        std::vector<uint8_t> objBytes = wasm::WasmModule::compile(msg);
        std::string objFilePath = util::getFunctionObjectFile(msg);
        util::writeBytesToFile(objFilePath, objBytes);
    }

    /**
     * Parse the WASM file to work out functions, exports, imports etc.
     */
    void WasmModule::parseWasm(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::vector<U8> fileBytes;
        std::string filePath = util::getFunctionFile(msg);

        if (!loadFile(filePath.c_str(), fileBytes)) {
            logger->error("Could not read data from {}", filePath);
            throw std::runtime_error("Could not read binary data from file");
        }

        WASM::loadBinaryModule(fileBytes.data(), fileBytes.size(), this->module);

        // Set up minimum memory size
        this->module.memories.defs[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;

        // Add the shared memory definition
        // this->module.memories.imports.push_back({{true, {0, 1000}}, msg.user(), "shared_state"});
    }

    U32 WasmModule::mmap(U32 length) {
        // Work out how many WAVM pages need to be added
        Uptr pagesRequested = getNumberOfPagesForBytes(length);

        Iptr previousPageCount = growMemory(defaultMemory, pagesRequested);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Growing memory from {} to {} WAVM pages", previousPageCount, previousPageCount + pagesRequested);

        if (previousPageCount == -1) {
            logger->error("No memory for mapping");
            throw std::runtime_error("Run out of memory to map");
        }

        // Get pointer to mapped range
        auto mappedRangePtr = (U32) (Uptr(previousPageCount) * IR::numBytesPerPage);

        return mappedRangePtr;
    }

    U32 WasmModule::mmapKey(state::StateKeyValue *kv, U32 length) {
        // See if we need to initialise this mapping or if it already exists
        if (sharedMemWasmPtrs.count(kv->key) == 0) {
            // Create memory region for this module
            U32 wasmPtr = this->mmap(length);

            // Do the mapping from the central shared region
            U8 *hostMemPtr = &Runtime::memoryRef<U8>(defaultMemory, wasmPtr);
            if (!util::isPageAligned(hostMemPtr)) {
                throw std::runtime_error("WAVM memory not page aligned");
            }

            void *voidPtr = static_cast<void *>(hostMemPtr);
            kv->mapSharedMemory(voidPtr);

            // Remember the kv and pointer
            sharedMemWasmPtrs.insert(std::pair<std::string, I32>(kv->key, wasmPtr));
            sharedMemHostPtrs.insert(std::pair<std::string, void *>(kv->key, voidPtr));
            sharedMemKVs.insert(std::pair<std::string, state::StateKeyValue *>(kv->key, kv));
        }

        // Return the wasm pointer
        return sharedMemWasmPtrs[kv->key];
    }

}