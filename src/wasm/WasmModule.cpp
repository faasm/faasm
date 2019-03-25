#include "WasmModule.h"

#include <memory/MemorySnapshotRegister.h>
#include <util/files.h>
#include <util/func.h>
#include <util/memory.h>
#include <wasm/FunctionLoader.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Inline/CLI.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <syscall.h>

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

    int expandMemory(U32 newSize) {
        Uptr targetPageCount = getNumberOfPagesForBytes(newSize);

        // Work out current size
        WasmModule *module = getExecutingModule();
        Runtime::Memory *memory = module->defaultMemory;
        const Uptr currentPageCount = getMemoryNumPages(memory);

        // Check if expanding too far
        Uptr maxPages = getMemoryMaxPages(memory);
        if (targetPageCount > maxPages) {
            return EXPAND_TOO_BIG;
        }

        // Nothing too be done if memory already big enough or new size is zero
        if (targetPageCount <= currentPageCount || newSize == 0) {
            return EXPAND_NO_ACTION;
        }

        Uptr expansion = targetPageCount - currentPageCount;

        // Grow memory as required
        growMemory(memory, expansion);

        return EXPAND_SUCCESS;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    U32 dynamicAllocString(Runtime::Memory *memory, const char *str, U32 len) {
        U32 wasmAddr = dynamicAlloc(memory, len);
        U8 *hostAddr = Runtime::memoryArrayPtr<U8>(memory, wasmAddr, len);
        memcpy(hostAddr, str, len);

        return wasmAddr;
    }

    void setEmscriptenDynamicTop(U32 newValue) {
        Runtime::Memory *memory = getExecutingModule()->defaultMemory;

        // Note that this MUST be a reference, otherwise we just update a copy that's not seen by wasm
        EmscriptenMutableGlobals &mutableGlobals = Runtime::memoryRef<EmscriptenMutableGlobals>(memory,
                                                                                                EmscriptenMutableGlobals::address);
        mutableGlobals.DYNAMICTOP_PTR = newValue;
    }

    U32 getEmscriptenDynamicTop() {
        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        EmscriptenMutableGlobals &mutableGlobals = Runtime::memoryRef<EmscriptenMutableGlobals>(memory,
                                                                                                EmscriptenMutableGlobals::address);
        return mutableGlobals.DYNAMICTOP_PTR;
    }

    /**
     * This is an emscripten-specific operation. The dyanamictop_ptr is used by emscripten
     * to keep track of where the top of its heap is. If dynamictop_ptr goes over the size of
     * the available memory, we need to expand the memory (and make sure we keep dynamictop_ptr
     * in sync.
     */
    U32 dynamicAlloc(Runtime::Memory *memory, U32 numBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Dynamically allocating emscripten memory for {} bytes", numBytes);

        // Shift the dynamic top ptr
        const U32 allocationAddress = getEmscriptenDynamicTop();
        const U32 endAddress = (allocationAddress + numBytes + 15) & -16;
        setEmscriptenDynamicTop(endAddress);

        const Uptr endPage = (endAddress + IR::numBytesPerPage - 1) / IR::numBytesPerPage;
        if (endPage >= getMemoryNumPages(memory) && endPage < getMemoryMaxPages(memory)) {
            growMemory(memory, endPage - getMemoryNumPages(memory) + 1);
        }

        return allocationAddress;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() {
        // Set all reference to GC pointers to null to allow WAVM GC to clear up
        defaultMemory = nullptr;
        defaultTable = nullptr;
        moduleInstance = nullptr;
        functionInstance = nullptr;

        for (auto const &m : dynamicModuleMap) {
            dynamicModuleMap[m.first] = nullptr;
        }

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

        memSnapshot.clear();
    };

    bool WasmModule::isBound() {
        return _isBound;
    }

    bool WasmModule::isEmscripten() {
        return resolver->isEmscripten;
    }

    bool WasmModule::isInitialised() {
        return _isInitialised;
    }

    void WasmModule::initialise() {
        if (compartment != nullptr) {
            throw std::runtime_error("Cannot initialise already initialised module");
        }

        compartment = Runtime::createCompartment();

        resolver = new RootResolver(compartment);

        _isInitialised = true;
    }


    Runtime::Function *WasmModule::getFunction(const std::string &funcName) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Look up the function
        Runtime::Function *func = asFunctionNullable(getInstanceExport(moduleInstance, funcName));
        if (!func) {
            logger->error("Unable to find function {}", funcName);
            throw std::runtime_error("Missing exported function");
        }

        return func;
    }

    void WasmModule::bindToFunction(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!_isInitialised) {
            throw std::runtime_error("Must initialise module before binding");
        } else if (_isBound) {
            throw std::runtime_error("Cannot bind a module twice");
        }

        resolver->setUser(msg.user());

        // Load the function data
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
        std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(msg);
        std::vector<uint8_t> objectFileBytes = functionLoader.loadFunctionObjectBytes(msg);

        // Create the module instance
        moduleInstance = this->createModuleInstance(module, wasmBytes, objectFileBytes, util::funcToString(msg), true);

        // Add this module to the resolver for dynamic imports
        resolver->setMainModule(moduleInstance);

        // Extract the module's exported function
        std::string entryFunc;
        if (this->isEmscripten()) {
            entryFunc = "_main";
        } else {
            entryFunc = "_start";
        }

        // Get main entrypoint function
        functionInstance = this->getFunction(entryFunc);

        // Keep reference to memory and table
        this->defaultMemory = Runtime::getDefaultMemory(moduleInstance);
        this->defaultTable = Runtime::getDefaultTable(moduleInstance);

        // Snapshot initial state
        logger->debug("Snapshotting {} pages of memory for restore", CLEAN_MEMORY_PAGES);
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        memSnapshot.createCopy(baseAddr, CLEAN_MEMORY_SIZE);

        // Record that this module is now bound
        _isBound = true;
        boundUser = msg.user();
        boundFunction = msg.function();
    }

    Runtime::ModuleInstance *
    WasmModule::createModuleInstance(IR::Module &irModule, const std::vector<uint8_t> &wasmBytes,
                                     const std::vector<uint8_t> &objBytes,
                                     const std::string &name, bool isMainModule) {
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

        if (functionLoader.isWasm(wasmBytes)) {
            WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), irModule);
        } else {
            std::vector<WAST::Error> parseErrors;
            WAST::parseModule((const char *) wasmBytes.data(), wasmBytes.size(), irModule, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);
        }

        // If this is the main module, set up the resolver
        if (isMainModule) {
            resolver->setUp(compartment, irModule);
        } else {
            // Check that the module isn't expecting to create any memories or tables
            if (!irModule.tables.defs.empty()) {
                throw std::runtime_error("Dynamic module trying to define tables");
            }

            if (!irModule.memories.defs.empty()) {
                throw std::runtime_error("Dynamic module trying to define memories");
            }
        }

        Runtime::LinkResult linkResult = linkModule(irModule, *resolver);
        if (!linkResult.success) {
            std::cerr << "Failed to link module:" << std::endl;
            throw std::runtime_error("Failed linking module");
        }

        // Instantiate the module, i.e. create memory, tables etc.
        Runtime::ModuleRef compiledModule;
        if (!objBytes.empty()) {
            compiledModule = Runtime::loadPrecompiledModule(irModule, objBytes);
        } else {
            compiledModule = Runtime::compileModule(irModule);
        }

        Runtime::ModuleInstance *instance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                name.c_str()
        );

        return instance;
    }

    I32 WasmModule::getGlobalI32(const std::string &globalName, Runtime::Context *context) {
        Runtime::Global *globalPtr = Runtime::asGlobal(Runtime::getInstanceExport(moduleInstance, globalName.c_str()));
        const IR::Value &value = Runtime::getGlobalValue(context, globalPtr);
        return value.i32;
    }

    int WasmModule::dynamicLoadModule(const std::string &path, Runtime::Context *context) {
        // Return the handle if we've already loaded this module
        if (dynamicPathToHandleMap.count(path) > 0) {
            return dynamicPathToHandleMap[path];
        }

        IR::Module sharedModule;
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

        // Get path to where machine code should be
        std::string objFilePath = path + SHARED_OBJ_EXT;
        std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(path);
        std::vector<uint8_t> objectBytes = functionLoader.loadFunctionObjectBytes(objFilePath);

        // Note, must start handles at 2, otherwise dlopen can see it as an error
        int nextHandle = 2 + dynamicModuleCount;
        std::string name = "handle_" + std::to_string(nextHandle);

        // TODO - how do we properly give the dynamic lib the right stack pointer?
        // It seems impossible to get the right stack pointer value from the module
        // It's assigned to a different global every time and isn't labelled
        // For now we can hack it to give it enough space
        int stackSize = 1 * ONE_MB_PAGES;
        int totalModuleMemSize = 4 * ONE_MB_PAGES;

        if (dynamicModuleCount == 0) {
            // Heap base is top of the stack, and data end is the bottom of the stack
            I32 heapBase = this->getGlobalI32("__heap_base", context);
            I32 dataEnd = this->getGlobalI32("__data_end", context);

            logger->debug("Linking first dynamic module. heap_base={} and data_end={}", heapBase, dataEnd);

            // Stack has a region just at the bottom of the empty region (and grows down)
            // Memory then sits above that (and grows up)
            resolver->nextStackPointer = heapBase + stackSize;
            resolver->nextMemoryBase = resolver->nextStackPointer + 1;

            // TODO - what should the table base be?
            Uptr currentTableElems = Runtime::getTableNumElements(defaultTable);
            resolver->nextTableBase = currentTableElems;
        } else {
            // Bump up memory base and table base from where they were last time
            // TODO - need to use dylink section to work out how big these jumps should be
            resolver->nextStackPointer += (totalModuleMemSize + stackSize);
            resolver->nextMemoryBase = resolver->nextStackPointer + 1;
            resolver->nextTableBase += 10;
        }

        // Instantiate the shared module
        Runtime::ModuleInstance *moduleInstance = createModuleInstance(
                sharedModule, wasmBytes, objectBytes, name, false
        );

        // Keep a record of this module
        dynamicPathToHandleMap[path] = nextHandle;
        dynamicModuleMap[nextHandle] = moduleInstance;
        dynamicModuleCount++;

        logger->debug("Loaded shared module at {} with handle {}", path, nextHandle);

        return nextHandle;
    }

    Uptr WasmModule::getDynamicModuleFunction(int handle, const std::string &funcName) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // TODO - avoid doing this if function is already in the table

        // Get main entrypoint function
        if (dynamicModuleMap.count(handle) == 0) {
            logger->error("No dynamic module registered for handle {}", handle);
            throw std::runtime_error("Missing dynamic module");
        }

        Runtime::ModuleInstance *dynModule = dynamicModuleMap[handle];

        // Something puts an underscore in front of the name
        Runtime::Object *exportedFunc = getInstanceExport(dynModule, funcName);

        if (!exportedFunc) {
            logger->error("Unable to dynamically load function {}", funcName);
            throw std::runtime_error("Missing dynamic module function");
        }

        // Add function to the table
        int nElements = 1;
        Iptr prevIdx = Runtime::growTable(defaultTable, nElements);
        if (prevIdx == -1) {
            throw std::runtime_error("Failed to grow table");
        } else {
            logger->debug("Growing table from {} elements to {}", prevIdx, prevIdx + nElements);
        }

        Runtime::setTableElement(defaultTable, prevIdx, exportedFunc);

        return prevIdx;
    }

    void WasmModule::snapshotFullMemory(const char *key) {
        // Get memory and dimensions
        Uptr currentPages = Runtime::getMemoryNumPages(this->defaultMemory);
        Uptr memSize = currentPages * IR::numBytesPerPage;
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);

        // Create the snapshot
        memory::MemorySnapshotRegister &snapRegister = memory::getGlobalMemorySnapshotRegister();
        const std::shared_ptr<memory::MemorySnapshot> snapshot = snapRegister.getSnapshot(key);
        snapshot->create(key, baseAddr, memSize);
    }

    void WasmModule::restoreFullMemory(const char *key) {
        // Retrieve the snapshot
        memory::MemorySnapshotRegister &snapRegister = memory::getGlobalMemorySnapshotRegister();
        const std::shared_ptr<memory::MemorySnapshot> snapshot = snapRegister.getSnapshot(key);

        // Resize memory accordingly
        size_t wasmPages = (snapshot->getSize() + IR::numBytesPerPage - 1) / IR::numBytesPerPage;
        this->resizeMemory(wasmPages);

        // Do the restore
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        snapshot->restore(baseAddr);
    }

    void WasmModule::resizeMemory(size_t targetPages) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Get current size
        Uptr currentPages = Runtime::getMemoryNumPages(this->defaultMemory);

        if (currentPages > targetPages) {
            Uptr shrinkSize = currentPages - targetPages;
            logger->debug("Shrinking memory by {} pages", shrinkSize);

            Runtime::shrinkMemory(this->defaultMemory, shrinkSize);

        } else if (targetPages > currentPages) {
            Uptr growSize = targetPages - currentPages;
            logger->debug("Growing memory by {} pages", growSize);

            Runtime::growMemory(this->defaultMemory, growSize);
        }
    }

    void WasmModule::restoreMemory() {
        this->resizeMemory(INITIAL_MEMORY_PAGES);

        // Restore initial memory in clean region
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        memSnapshot.restoreCopy(baseAddr);

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

        // Set up the call and invoke arguments
        Runtime::Context *context = Runtime::createContext(compartment);
        std::vector<IR::Value> invokeArgs;

        if (this->isEmscripten()) {
            U8 *memoryBaseAddress = getMemoryBaseAddress(defaultMemory);
            U32 *argvOffsets = (U32 *) (memoryBaseAddress + dynamicAlloc(defaultMemory, (U32) (sizeof(U32))));
            argvOffsets[0] = 0;
            invokeArgs = {(U32) 0, (U32) ((U8 *) argvOffsets - memoryBaseAddress)};

            // Call the global initializer functions.
            for (Uptr exportIndex = 0; exportIndex < module.exports.size(); ++exportIndex) {
                const IR::Export &functionExport = module.exports[exportIndex];

                int isGlobal = strncmp(functionExport.name.c_str(), "__GLOBAL__", 10);
                bool isFunction = functionExport.kind == IR::ExternKind::function;

                if (isFunction && !isGlobal) {
                    Runtime::Function *function = asFunctionNullable(
                            getInstanceExport(moduleInstance, functionExport.name));
                    if (function) {
                        logger->info("Executing Emscripten global initialiser: {}", functionExport.name.c_str());
                        Runtime::invokeFunctionChecked(context, function, {});
                    }
                }
            }

            // Store ___errno_location.
            Runtime::Function *errNoLocation
                    = asFunctionNullable(getInstanceExport(moduleInstance, "___errno_location"));
            if (errNoLocation
                && getFunctionType(errNoLocation) ==
                   IR::FunctionType(IR::TypeTuple{IR::ValueType::i32}, IR::TypeTuple{})) {
                IR::ValueTuple errNoResult = Runtime::invokeFunctionChecked(context, errNoLocation, {});
                if (errNoResult.size() == 1 && errNoResult[0].type == IR::ValueType::i32) {
                    throw std::runtime_error("Not supporting errno location");
                }
            }
        }

        int exitCode = 0;
        try {
            // Call the function
            logger->debug("Invoking the function itself");
            Runtime::unwindSignalsAsExceptions([this, &context, &invokeArgs] {
                invokeFunctionChecked(context, functionInstance, invokeArgs);
            });
        }
        catch (wasm::WasmExitException &e) {
            exitCode = e.exitCode;
        }
        catch (Runtime::Exception *ex) {
            logger->error("Runtime exception: {}", Runtime::describeException(ex).c_str());
            Runtime::destroyException(ex);
        }

        return exitCode;
    }

    U32 WasmModule::mmap(U32 length) {
        // Round up to page boundary
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

    U32 WasmModule::mmapKey(const std::shared_ptr<state::StateKeyValue> kv, U32 length) {
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
            sharedMemKVs.insert(std::pair<std::string, std::shared_ptr<state::StateKeyValue>>(kv->key, kv));
        }

        // Return the wasm pointer
        return sharedMemWasmPtrs[kv->key];
    }

}