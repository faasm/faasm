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

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() {
        // Set all reference to GC pointers to null to allow WAVM GC to clear up
        defaultMemory = nullptr;
        defaultTable = nullptr;
        moduleInstance = nullptr;
        functionInstance = nullptr;
        envModule = nullptr;

        for (auto const &m : dynamicModuleMap) {
            dynamicModuleMap[m.first] = nullptr;
        }

        if (compartment != nullptr) {
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

    bool WasmModule::isInitialised() {
        return _isInitialised;
    }

    void WasmModule::initialise() {
        if (compartment != nullptr) {
            throw std::runtime_error("Cannot initialise already initialised module");
        }

        compartment = Runtime::createCompartment();

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

    void WasmModule::addModuleToGOT(IR::Module &mod, bool isMainModule) {
        // Retrieve the disassembly names to help with building the GOT
        IR::DisassemblyNames disassemblyNames;
        getDisassemblyNames(mod, disassemblyNames);

        // Keep track of which functions are in the GOT
        std::unordered_set<std::string> functionsAdded;

        // ----------------------------
        // Function entries
        // ----------------------------
        // Function entries in main modules are specified in the elem sections in order, so we can
        // iterate through this to get their table indices
        for (auto &es : mod.elemSegments) {
            // Work out the offset (dynamic modules will be given this by us)
            I32 offset;
            if (isMainModule) {
                offset = es.baseOffset.i32;
            } else {
                offset = nextTableBase;
            }

            // Go through each elem entry and record where in the table it's getting inserted
            for (Uptr i = 0; i < es.elems.size(); i++) {
                const IR::Elem &elem = es.elems[i];

                // Ignore null entries
                if (!es.isActive && elem.type == IR::Elem::Type::ref_null) {
                    continue;
                }

                std::string &elemName = disassemblyNames.functions[elem.index].name;
                int tableIdx = offset + i;
                globalOffsetTableMap.insert({elemName, tableIdx});
                functionsAdded.insert(elemName);
            }
        }

        // ----------------------------
        // Data entries
        // ----------------------------
        // Here we need to build up a map of the export's name to its initialised value.
        // We assume the entries we care about are pointers to stuff in memory, therefore we
        // only look at immutable i32s
        for (auto &ex : mod.exports) {
            // Ignore non global exports
            if (ex.kind != IR::ExternKind::global) {
                continue;
            }

            // Get the global definition for this export
            int i = ex.index;
            const IR::GlobalDef &global = mod.globals.getDef(i);

            // Skip if mutable or not I32
            if (global.type.isMutable || global.initializer.type != IR::InitializerExpression::Type::i32_const) {
                continue;
            }

            // Add the initialised value to the map
            I32 value = nextMemoryBase + global.initializer.i32;
            globalOffsetMemoryMap.insert({ex.name, value});
        }
    }

    void WasmModule::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            throw std::runtime_error("Must initialise module before binding");
        } else if (_isBound) {
            throw std::runtime_error("Cannot bind a module twice");
        }

        // Load the function data
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
        std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(msg);
        std::vector<uint8_t> objectFileBytes = functionLoader.loadFunctionObjectBytes(msg);

        // Create the module instance
        moduleInstance = this->createModuleInstance(module, wasmBytes, objectFileBytes, util::funcToString(msg), true);

        // Extract the module's exported function
        std::string entryFunc = "main";

        // Get main entrypoint function
        functionInstance = this->getFunction(entryFunc);

        // Keep reference to memory and table
        this->defaultMemory = Runtime::getDefaultMemory(moduleInstance);
        this->defaultTable = Runtime::getDefaultTable(moduleInstance);

        // Snapshot initial state
//        logger->debug("Snapshotting {} pages of memory for restore", CLEAN_MEMORY_PAGES);
//        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
//        memSnapshot.createCopy(baseAddr, CLEAN_MEMORY_SIZE);

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

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (functionLoader.isWasm(wasmBytes)) {
            WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), irModule);
        } else {
            std::vector<WAST::Error> parseErrors;
            WAST::parseModule((const char *) wasmBytes.data(), wasmBytes.size(), irModule, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);
        }

        if (isMainModule) {
            // Force memory sizes
            // module.memories.defs[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;
            module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;

            // Note, we don't want to mess with the min table size here, just give it room to expand if need be
            module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;

            // Set up intrinsics
            envModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_env(), "env");
        } else {
            // Check that the module isn't expecting to create any memories or tables
            if (!irModule.tables.defs.empty()) {
                throw std::runtime_error("Dynamic module trying to define tables");
            }

            if (!irModule.memories.defs.empty()) {
                throw std::runtime_error("Dynamic module trying to define memories");
            }

            // Extend the existing table to fit all the dynamic module's elements
            U64 nTableElems = irModule.tables.imports[0].type.size.min;
            logger->debug("Growing table to fit {} dynamic module elements", nTableElems);

            Iptr oldTableElems = Runtime::growTable(defaultTable, nTableElems);
            if (oldTableElems == -1) {
                throw std::runtime_error("Failed to grow main module table");
            }

            // Now force the incoming dynamic module to accept the table from the main module
            irModule.tables.imports[0].type.size.min = (U64) module.tables.defs[0].type.size.min;
            irModule.tables.imports[0].type.size.max = (U64) module.tables.defs[0].type.size.max;
        }

        // Add module to GOT before linking
        addModuleToGOT(irModule, isMainModule);

        // Do the linking
        Runtime::LinkResult linkResult = linkModule(irModule, *this);
        if (!linkResult.success) {
            logger->error("Failed to link module");
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

        // Attempt to fill gaps in missing table entries - loading module can fail here
        if(missingGlobalOffsetEntries.size() > 0) {
            for(auto e : missingGlobalOffsetEntries) {
                Runtime::Object *missingFunction = getInstanceExport(instance, e.first);

                if(!missingFunction) {
                    logger->error("Could not fill gap in GOT for function: {}", e.first);
                    throw std::runtime_error("Failed linking module on missing GOT entry");
                }

                // Fill in the empty placeholder
                logger->debug("Filling gap in GOT for function: {} at {}", e.first, e.second);
                Runtime::setTableElement(defaultTable, e.second, missingFunction);
            }
        }
        
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

        // Create a new region in the default memory 
        U32 dynamicMemBase = this->mmap(DYNAMIC_MODULE_HEAP_SIZE);

        // Give the module a stack region just at the bottom of the empty region (which will grow down)
        // Memory sits above that (and grows up).
        // TODO - how do we detect stack overflows here? Are we meant to share the stack pointer of the main module?
        U32 dynamicHeapBase = dynamicMemBase + DYNAMIC_MODULE_STACK_SIZE;
        U32 dynamicStackBase = dynamicHeapBase - 1;

        this->nextStackPointer = dynamicStackBase;
        this->nextMemoryBase = dynamicHeapBase;

        // The end of the current table is the place where the new module can put its elements.
        Uptr currentTableElems = Runtime::getTableNumElements(defaultTable);
        this->nextTableBase = currentTableElems;

        logger->debug("Provisioned new dynamic module region (stack_ptr={}, heap_base={}, table_base={}",
                      this->nextStackPointer,
                      this->nextMemoryBase,
                      this->nextTableBase
        );

        // Instantiate the shared module
        Runtime::ModuleInstance *mod = createModuleInstance(
                sharedModule, wasmBytes, objectBytes, name, false
        );

        // Call the wasm constructor function. This allows the module to perform any relevant initialisation
        Runtime::Function *wasmCtorsFunction = asFunctionNullable(getInstanceExport(mod, "__wasm_call_ctors"));
        if (wasmCtorsFunction) {
            logger->debug("Executing __wasm_call_ctors for {}", path);
            Runtime::invokeFunctionChecked(context, wasmCtorsFunction, {});
        }

        // Keep a record of this module
        dynamicPathToHandleMap[path] = nextHandle;
        dynamicModuleMap[nextHandle] = mod;
        dynamicModuleCount++;

        logger->debug("Loaded shared module at {} with handle {}", path, nextHandle);

        return nextHandle;
    }

    Uptr WasmModule::getDynamicModuleFunction(int handle, const std::string &funcName) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check the handle is valid
        if (dynamicModuleMap.count(handle) == 0) {
            logger->error("No dynamic module registered for handle {}", handle);
            throw std::runtime_error("Missing dynamic module");
        }

        // If function not in table for some reason, we need to load it
        Runtime::ModuleInstance *dynModule = dynamicModuleMap[handle];
        Runtime::Object *exportedFunc = getInstanceExport(dynModule, funcName);

        if (!exportedFunc) {
            logger->error("Unable to dynamically load function {}", funcName);
            throw std::runtime_error("Missing dynamic module function");
        }

        int prevIdx = this->addFunctionToTable(exportedFunc);
        return prevIdx;
    }

    int WasmModule::addFunctionToTable(Runtime::Object *exportedFunc) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Add function to the table
        Iptr prevIdx = Runtime::growTable(defaultTable, 1);
        if (prevIdx == -1) {
            throw std::runtime_error("Failed to grow table");
        } else {
            logger->debug("Growing table from {} elements to {}", prevIdx, prevIdx + 1);
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
        if (initialMemoryPages == 0) {
            throw std::runtime_error("Initial memory pages not initialised");
        }

        // Shrink back to original size
        this->resizeMemory(initialMemoryPages);

        // Restore initial memory in clean region
//        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
//        memSnapshot.restoreCopy(baseAddr);

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

    int WasmModule::getInitialMemoryPages() {
        return initialMemoryPages;
    }

    int WasmModule::getHeapBase() {
        return heapBase;
    }

    int WasmModule::getDataEnd() {
        return dataEnd;
    }

    int WasmModule::getStackTop() {
        return stackTop;
    }

    void WasmModule::setErrno(int newValue) {
        if (errnoLocation != 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->debug("Setting errno={} ({}) (errno_location={})", newValue, strerror(newValue), errnoLocation);

            Runtime::memoryRef<I32>(defaultMemory, errnoLocation) = (I32) newValue;
        } else {
            throw std::runtime_error("Attempting to set errno without errno location set");
        }
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

        // Set up the context
        Runtime::Context *context = Runtime::createContext(compartment);

        // Memory-related variables (these will be the same when stack first is switched on)
        initialMemoryPages = Runtime::getMemoryNumPages(defaultMemory);
        Uptr initialMemorySize = initialMemoryPages * IR::numBytesPerPage;

        heapBase = this->getGlobalI32("__heap_base", context);
        dataEnd = this->getGlobalI32("__data_end", context);
        logger->debug("heap_base = {}  data_end = {}  heap_top={}", heapBase, dataEnd, initialMemorySize);

        // The stack top variable should be the first global
        IR::GlobalDef stackDef = module.globals.getDef(0);
        if (!stackDef.type.isMutable) {
            throw std::runtime_error("Found immutable stack top");
        }

        stackTop = stackDef.initializer.i32;

        // Set up invoke arguments just below the top of the memory (i.e. at the top of the dynamic section)
        U32 argvStart = initialMemorySize - 20;
        U32 argc = 0;

        // Copy the function name into argv[0]
        U32 argv0 = argvStart + sizeof(U32);
        char *argv0Host = &Runtime::memoryRef<char>(defaultMemory, argv0);
        strcpy(argv0Host, "function.wasm");

        // Copy the offset of argv[0] into position
        Runtime::memoryRef<U32>(defaultMemory, argvStart) = argv0;
        std::vector<IR::Value> invokeArgs = {argc, argvStart};

        // Record the errno location
        Runtime::Function *errNoLocation = asFunctionNullable(getInstanceExport(moduleInstance, "___errno_location"));
        if (errNoLocation) {
            IR::ValueTuple errNoResult = Runtime::invokeFunctionChecked(context, errNoLocation, {});
            if (errNoResult.size() == 1 && errNoResult[0].type == IR::ValueType::i32) {
                errnoLocation = errNoResult[0].i32;
                logger->debug("Found errno location {}", errnoLocation);
            }
        } else {
            errnoLocation = ERRNO_ADDR;
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

    I32 WasmModule::sBrk(U32 increment) {
        Uptr currentBrk = Runtime::getMemoryNumPages(defaultMemory) * IR::numBytesPerPage;
        if (increment == 0) {
            // Calling with zero is the same as calling brk with zero,
            // return the current break
            return currentBrk;
        }

        U32 target = currentBrk + increment;
        int brkResult = this->brk(target);

        if (brkResult == EXPAND_SUCCESS) {
            // Return the new break
            Uptr newBrk = Runtime::getMemoryNumPages(defaultMemory) * IR::numBytesPerPage;
            return newBrk;
        } else {
            return -1;
        }
    }

    I32 WasmModule::brk(U32 newSize) {
        Uptr targetPageCount = getNumberOfPagesForBytes(newSize);

        // Work out current size
        const Uptr currentPageCount = getMemoryNumPages(defaultMemory);

        // Check if expanding too far
        Uptr maxPages = getMemoryMaxPages(defaultMemory);
        if (targetPageCount > maxPages) {
            return EXPAND_TOO_BIG;
        }

        // Nothing too be done if memory already big enough or new size is zero
        if (targetPageCount <= currentPageCount || newSize == 0) {
            return EXPAND_NO_ACTION;
        }

        Uptr expansion = targetPageCount - currentPageCount;

        // Grow memory as required
        Iptr prevPageCount = growMemory(defaultMemory, expansion);
        if (prevPageCount == -1) {
            throw std::runtime_error("Something has gone seriously wrong with brk");
        }

        return EXPAND_SUCCESS;
    }

    bool WasmModule::resolve(const std::string &moduleName,
                             const std::string &name,
                             IR::ExternType type,
                             Runtime::Object *&resolved) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        bool isMainModule = moduleInstance == nullptr;

        if (isMainModule) {
            // Main module linking comes from env module
            resolved = getInstanceExport(envModule, name);
        } else {
            if (moduleName == "GOT.mem") {
                // Handle global offset table memory entries
                if (globalOffsetMemoryMap.count(name) == 0) {
                    logger->error("Failed to look up memory offset in GOT: {}.{}", moduleName, name);
                    return false;
                }

                int memOffset = globalOffsetMemoryMap[name];
                logger->debug("Resolved {}.{} to {}", moduleName, name, memOffset);

                // Create a global to hold the offset value
                Runtime::Global *gotMemoryOffset = Runtime::createGlobal(compartment, asGlobalType(type));
                Runtime::initializeGlobal(gotMemoryOffset, memOffset);
                resolved = asObject(gotMemoryOffset);

            } else if (moduleName == "GOT.func") {
                int tableIdx = -1;

                // See if it's already in the GOT
                if (globalOffsetTableMap.count(name) > 0) {
                    tableIdx = globalOffsetTableMap[name];
                    logger->debug("Resolved {}.{} to offset {}", moduleName, name, tableIdx);
                }

                // Look in other imported modules if not found
                if (tableIdx == -1) {
                    for (auto m : dynamicModuleMap) {
                        Runtime::Object *resolvedFunc = getInstanceExport(m.second, name);
                        if (resolvedFunc) {
                            // Add the function to the map
                            tableIdx = this->addFunctionToTable(resolvedFunc);
                            globalOffsetTableMap.insert({name, tableIdx});
                            break;
                        }
                    }
                }

                // If not found, create a placeholder to be filled in later
                // TODO - what causes this? The module both exports _and_ has a GOT entry for the function?
                if (tableIdx == -1) {
                    // Create a new entry in the table and use this, but mark it to be filled later
                    tableIdx = Runtime::growTable(defaultTable, 1);
                    logger->warn("Adding placeholder table offset: {}.{} at {}", moduleName, name, tableIdx);
                    missingGlobalOffsetEntries.insert({name, tableIdx});
                }

                // Create a global to hold the function offset
                Runtime::Global *gotFunctionOffset = Runtime::createGlobal(compartment, asGlobalType(type));
                Runtime::initializeGlobal(gotFunctionOffset, tableIdx);
                resolved = asObject(gotFunctionOffset);

            } else if (name == "__memory_base") {
                // Memory base tells the loaded module where to start its heap
                Runtime::Global *newMemoryBase = Runtime::createGlobal(compartment, asGlobalType(type));
                Runtime::initializeGlobal(newMemoryBase, nextMemoryBase);
                resolved = asObject(newMemoryBase);

            } else if (name == "__table_base") {
                // Table base tells the loaded module where to start its table entries
                Runtime::Global *newTableBase = Runtime::createGlobal(compartment, asGlobalType(type));
                Runtime::initializeGlobal(newTableBase, nextTableBase);
                resolved = asObject(newTableBase);

            } else if (name == "__stack_pointer") {
                // Stack pointer is where the loaded module should put its stack
                Runtime::Global *newStackPointer = Runtime::createGlobal(compartment, asGlobalType(type));
                Runtime::initializeGlobal(newStackPointer, nextStackPointer);
                resolved = asObject(newStackPointer);

            } else if (name == "__indirect_function_table") {
                // This is the name for the table imported from the main module
                Runtime::Table *table = Runtime::getDefaultTable(moduleInstance);
                resolved = asObject(table);
            } else {
                // First check in normal env
                resolved = getInstanceExport(envModule, name);

                // Check the main module if not
                if (!resolved) {
                    resolved = getInstanceExport(moduleInstance, name);
                }

                // Check other dynamically loaded modules for the export
                if (!resolved) {
                    for (auto &m : dynamicModuleMap) {
                        resolved = getInstanceExport(m.second, name);
                        if (resolved) {
                            break;
                        }
                    }
                }
            }
        }

        // Check whether the target has been resolved to the correct type
        if (resolved) {
            if (isA(resolved, type)) {
                return true;
            } else {
                logger->error("Resolved import {}.{} to a {}, but was expecting {}",
                              moduleName.c_str(),
                              name.c_str(),
                              asString(getObjectType(resolved)).c_str(),
                              asString(type).c_str());
                return false;
            }
        }

        logger->error("Missing import {}.{} {}", moduleName, name, asString(type).c_str());

        return false;
    }
}