#include "WasmModule.h"

#include <util/files.h>
#include <util/func.h>
#include <util/memory.h>
#include <util/config.h>
#include <util/locks.h>

#include <memory/MemorySnapshotRegister.h>
#include <wasm/IRModuleRegistry.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/WASTParse/WASTParse.h>


using namespace WAVM;

namespace wasm {
    static thread_local WasmModule *executingModule;
    static thread_local message::Message *executingCall;
    static thread_local std::set<int> openFds;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    message::Message *getExecutingCall() {
        return executingCall;
    }

    std::string snapshotKeyForFunction(const std::string &user, const std::string &func) {
        return "mem_" + user + "_" + func;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    WasmModule::WasmModule() = default;

    WasmModule &WasmModule::operator=(const WasmModule &other) {
        throw std::runtime_error("Copy assignment not supported");
    }

    WasmModule::WasmModule(const WasmModule &other) {
        cloneFrom(other);
    }

    void WasmModule::cloneFrom(const WasmModule &other) {
        // Copy basic values
        errnoLocation = other.errnoLocation;
        initialMemoryPages = other.initialMemoryPages;
        initialTableSize = other.initialTableSize;
        heapBase = other.heapBase;
        dataEnd = other.dataEnd;
        stackTop = other.stackTop;

        dynamicModuleCount = other.dynamicModuleCount;
        nextMemoryBase = other.nextMemoryBase;
        nextStackPointer = other.nextStackPointer;
        nextTableBase = other.nextTableBase;

        _isInitialised = other._isInitialised;
        _isBound = other._isBound;
        boundUser = other.boundUser;
        boundFunction = other.boundFunction;

        // Copy module-specific info
        if (other._isInitialised) {
            // Clone compartment (covers all WAVM internals)
            compartment = Runtime::cloneCompartment(other.compartment);
        }

        if (other._isBound) {
            // Remap bits we need references to
            envModule = Runtime::remapToClonedCompartment(other.envModule, compartment);
            moduleInstance = Runtime::remapToClonedCompartment(other.moduleInstance, compartment);
            functionInstance = Runtime::remapToClonedCompartment(other.functionInstance, compartment);

            // Extract the memory and table again
            this->defaultMemory = Runtime::getDefaultMemory(moduleInstance);
            this->defaultTable = Runtime::getDefaultTable(moduleInstance);

            // TODO - include shared memory state

            // Remap dynamic modules
            // TODO - double check this works
            dynamicPathToHandleMap = other.dynamicPathToHandleMap;
            for (auto &p : other.dynamicModuleMap) {
                Runtime::ModuleInstance *newInstance = Runtime::remapToClonedCompartment(p.second, compartment);
                dynamicModuleMap[p.first] = newInstance;
            }

            // Copy dynamic linking stuff
            globalOffsetTableMap = other.globalOffsetTableMap;
            globalOffsetMemoryMap = other.globalOffsetMemoryMap;
            missingGlobalOffsetEntries = other.missingGlobalOffsetEntries;
        }
    }

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
    }

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

        // ----------------------------
        // Table elems
        // ----------------------------
        // A lot of function entries in main modules are already included in the table,
        // so we can iterate through the elem segments to work out their index
        for (auto &es : mod.elemSegments) {
            // Work out the offset (dynamic modules will be given this by us)
            I32 offset;
            if (isMainModule) {
                offset = es.baseOffset.i32;
            } else {
                offset = nextTableBase;
            }

            // Go through each elem entry and record where in the table it's getting inserted
            for (Uptr i = 0; i < es.elems->size(); i++) {
                const IR::Elem &elem = (*es.elems)[i];

                // Ignore null entries
                if (!es.isActive && elem.type == IR::Elem::Type::ref_null) {
                    continue;
                }

                std::string &elemName = disassemblyNames.functions[elem.index].name;
                int tableIdx = offset + i;
                globalOffsetTableMap.insert({elemName, tableIdx});
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

        // Record that this module is now bound
        _isBound = true;
        boundUser = msg.user();
        boundFunction = msg.function();

        // TODO - tidy up the handling of Python functions. This is a hack
        // Create a copy of the message to avoid messing with the original
        message::Message msgCopy = msg;

        if (msg.ispython()) {
            std::string pyFile = msg.user() + "/" + msg.function() + ".py";
            msgCopy.set_inputdata(pyFile);
            msgCopy.set_user("python");
            msgCopy.set_function("py_func");
        }

        // Create the module instance
        moduleInstance = this->createModuleInstance(util::funcToString(msgCopy), "");

        // Extract the module's exported function
        std::string entryFunc = "main";

        // Get main entrypoint function
        functionInstance = this->getFunction(entryFunc);

        // Keep reference to memory and table
        this->defaultMemory = Runtime::getDefaultMemory(moduleInstance);
        this->defaultTable = Runtime::getDefaultTable(moduleInstance);

        // Memory-related variables
        initialMemoryPages = Runtime::getMemoryNumPages(defaultMemory);
        initialTableSize = Runtime::getTableNumElements(defaultTable);
        Uptr initialMemorySize = Runtime::getMemoryNumPages(defaultMemory) * IR::numBytesPerPage;

        // Get memory and dimensions
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);

        // Create the snapshot if not already present
        std::string snapKey = snapshotKeyForFunction(this->boundUser, this->boundFunction);
        memory::MemorySnapshotRegister &snapRegister = memory::getGlobalMemorySnapshotRegister();
        const std::shared_ptr<memory::MemorySnapshot> snapshot = snapRegister.getSnapshot(snapKey);
        snapshot->createIfNotExists(snapKey.c_str(), baseAddr, initialMemorySize);
    }

    void WasmModule::snapshot(const char *key) {
        WasmModuleRegistry &registry = wasm::getWasmModuleRegistry();
        registry.registerModule(key, *this);
    }

    void WasmModule::restore(const char *key) {
        WasmModuleRegistry &registry = wasm::getWasmModuleRegistry();
        WasmModule &otherRef = registry.getModule(key);

        this->cloneFrom(otherRef);
    }

    Runtime::ModuleInstance *
    WasmModule::createModuleInstance(const std::string &name, const std::string &sharedModulePath) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        IRModuleRegistry &moduleRegistry = wasm::getIRModuleRegistry();
        bool isMainModule = sharedModulePath.empty();

        // Warning: be very careful here to stick to *references* to the same shared modules
        // rather than creating copies.
        IR::Module &irModule = moduleRegistry.getModule(this->boundUser, this->boundFunction, sharedModulePath);
        if (isMainModule) {
            // Set up intrinsics
            envModule = Intrinsics::instantiateModule(compartment, {INTRINSIC_MODULE_REF(env)}, "env");

            // Make sure the stack top is as expected
            IR::GlobalDef stackDef = irModule.globals.getDef(0);
            if (!stackDef.type.isMutable) {
                throw std::runtime_error("Found immutable stack top");
            }

            stackTop = stackDef.initializer.i32;
        } else {
            // Create a new region in the default memory
            // Give the module a stack region just at the bottom of the empty region (which will grow down)
            // Memory sits above that (and grows up).
            // TODO - how do we detect stack overflows in dynamic modules? Are we meant to share the stack pointer of the main module?
            U32 dynamicMemBase = this->mmap(DYNAMIC_MODULE_HEAP_SIZE);
            this->nextMemoryBase = dynamicMemBase + DYNAMIC_MODULE_STACK_SIZE;
            this->nextStackPointer = this->nextMemoryBase - 1;

            // Extend the existing table to fit all the dynamic module's elements
            U64 nTableElems = moduleRegistry.getSharedModuleTableSize(this->boundUser, this->boundFunction,
                                                                      sharedModulePath);
            Iptr oldTableElems = Runtime::growTable(defaultTable, nTableElems);
            if (oldTableElems == -1) {
                throw std::runtime_error("Failed to grow main module table");
            }
            Uptr newTableElems = Runtime::getTableNumElements(defaultTable);

            // The end of the current table is the place where the new module can put its elements.
            this->nextTableBase = oldTableElems;

            logger->debug("Provisioned new dynamic module region (stack_ptr={}, heap_base={}, table={}->{})",
                          this->nextStackPointer,
                          this->nextMemoryBase,
                          this->nextTableBase,
                          newTableElems
            );
        }

        // Add module to GOT before linking
        addModuleToGOT(irModule, isMainModule);

        // Do the linking
        Runtime::LinkResult linkResult = linkModule(irModule, *this);
        if (!linkResult.success) {
            logger->error("Failed to link module");
            throw std::runtime_error("Failed linking module");
        }

        Runtime::ModuleRef compiledModule = moduleRegistry.getCompiledModule(this->boundUser, this->boundFunction,
                                                                             sharedModulePath);
        Runtime::ModuleInstance *instance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                name.c_str()
        );

        // Attempt to fill gaps in missing table entries. This is *only* to see if the missing
        // entry is exported from the dynamic module itself. I don't know how this happens but occasionally
        // it does
        if (missingGlobalOffsetEntries.size() > 0) {
            for (auto e : missingGlobalOffsetEntries) {
                // Check if it's an export of the module we're currently importing
                Runtime::Object *missingFunction = getInstanceExport(instance, e.first);

                if (!missingFunction) {
                    logger->error("Could not fill gap in GOT for function: {}", e.first);
                    throw std::runtime_error("Failed linking module on missing GOT entry");
                }

                // Put the actual function into the placeholder table location
                logger->debug("Filling gap in GOT for function: {} at {}", e.first, e.second);
                Runtime::setTableElement(defaultTable, e.second, missingFunction);

                // Add this function to the GOT
                globalOffsetTableMap.insert({e.first, e.second});
            }
        }

        // Empty the missing entries now that they're populated
        missingGlobalOffsetEntries.clear();

        return instance;
    }

    I32 WasmModule::getGlobalI32(const std::string &globalName, Runtime::Context *context) {
        Runtime::Global *globalPtr = Runtime::asGlobal(Runtime::getInstanceExport(moduleInstance, globalName.c_str()));
        const IR::Value &value = Runtime::getGlobalValue(context, globalPtr);
        return value.i32;
    }

    int WasmModule::dynamicLoadModule(const std::string &path, Runtime::Context *context) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Return the handle if we've already loaded this module
        if (dynamicPathToHandleMap.count(path) > 0) {
            logger->debug("Reusing dynamic module {}", path);
            return dynamicPathToHandleMap[path];
        }


        // Note, must start handles at 2, otherwise dlopen can see it as an error
        int nextHandle = 2 + dynamicModuleCount;
        std::string name = "handle_" + std::to_string(nextHandle);

        // Instantiate the shared module
        Runtime::ModuleInstance *mod = createModuleInstance(name, path);

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

    void WasmModule::reset() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Clean up any open fds
        this->clearFds();

        // Retrieve the memory snapshot
        memory::MemorySnapshotRegister &snapRegister = memory::getGlobalMemorySnapshotRegister();
        std::string key = snapshotKeyForFunction(this->boundUser, this->boundFunction);
        const std::shared_ptr<memory::MemorySnapshot> snapshot = snapRegister.getSnapshot(key);

        // Resize memory accordingly
        size_t wasmPages = (snapshot->getSize() + IR::numBytesPerPage - 1) / IR::numBytesPerPage;
        this->resizeMemory(wasmPages);

        // Do the restore
        U8 *baseAddr = Runtime::getMemoryBaseAddress(this->defaultMemory);
        snapshot->restore(baseAddr);

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
 
        // Restore the table to its original size
        Uptr currentTableSize = Runtime::getTableNumElements(defaultTable);
        Uptr elemsToShrink = currentTableSize - initialTableSize;
        if (elemsToShrink > 0) {
            logger->debug("Shrinking table from {} to {}", currentTableSize, initialTableSize);
            Runtime::shrinkTable(defaultTable, elemsToShrink);
        }

        // Remove references to shared modules
        for (auto const &p : dynamicPathToHandleMap) {
            logger->debug("Unloading dynamic module {}", p.first);
            dynamicModuleMap[p.second] = nullptr;
        }

        // Run WAVM GC
        Runtime::collectCompartmentGarbage(compartment);

        // Reset maps
        dynamicModuleMap.clear();
        dynamicPathToHandleMap.clear();
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

    Uptr WasmModule::getInitialMemoryPages() {
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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        if (errnoLocation != 0) {
            logger->debug("Setting errno={} ({}) (errno_location={})", newValue, strerror(newValue), errnoLocation);

            Runtime::memoryRef<I32>(defaultMemory, errnoLocation) = (I32) newValue;
        } else {
            logger->warn("No errno set but trying to set to {} ({})", strerror(newValue));
        }
    }

    /**
     * Executes the given function call
     */
    int WasmModule::execute(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(msg);

        if (!_isBound) {
            throw std::runtime_error("WorkerThread must be bound before executing function");
        } else if (boundUser != msg.user() || boundFunction != msg.function()) {
            logger->error("Cannot execute {} on module bound to {}/{}",
                          funcStr, boundUser, boundFunction);
            throw std::runtime_error("Cannot execute function on module bound to another");
        }

        // Set up shared references
        executingModule = this;
        executingCall = &msg;

        // Set up the context
        Runtime::Context *context = Runtime::createContext(compartment);

        // Memory-related variables
        Uptr initialMemorySize = initialMemoryPages * IR::numBytesPerPage;

        // Note that heap base and data end should be the same (provided stack-first is switched on)
        heapBase = this->getGlobalI32("__heap_base", context);
        dataEnd = this->getGlobalI32("__data_end", context);
        logger->debug("heap_base = {}  data_end = {}  heap_top={} initial_pages={}", heapBase, dataEnd,
                      initialMemorySize, initialMemoryPages);

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
        Runtime::Function *errNoLocation = asFunctionNullable(getInstanceExport(moduleInstance, "__errno_location"));
        if (errNoLocation) {
            IR::ValueTuple errNoResult = Runtime::invokeFunctionChecked(context, errNoLocation, {});
            if (errNoResult.size() == 1 && errNoResult[0].type == IR::ValueType::i32) {
                errnoLocation = errNoResult[0].i32;
                logger->debug("Found errno location {}", errnoLocation);
            }
        } else {
            logger->warn("Did not find errno location");
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
            exitCode = 1;
        }

        // Reset ready for next execution
        this->reset();

        return exitCode;
    }

    U32 WasmModule::mmap(U32 length) {
        // Round up to page boundary
        Uptr pagesRequested = getNumberOfPagesForBytes(length);

        Iptr previousPageCount = growMemory(defaultMemory, pagesRequested);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        Uptr newPageCount = previousPageCount + pagesRequested;
        if (previousPageCount == -1) {
            logger->error("No memory for mapping (growing to {} pages)", newPageCount);
            throw std::runtime_error("Run out of memory to map");
        }

        logger->debug("mmap - Growing memory from {} to {} pages", previousPageCount, newPageCount);

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

                // Check in already loaded modules if not found
                if (tableIdx == -1) {
                    // First check the main module
                    Runtime::Object *resolvedFunc = getInstanceExport(moduleInstance, name);

                    // Check other dynamic modules if not found in main module
                    if (!resolvedFunc) {
                        for (auto m : dynamicModuleMap) {
                            resolvedFunc = getInstanceExport(m.second, name);
                            if (resolvedFunc) {
                                break;
                            }
                        }
                    }

                    // If we've found something, add it to the table
                    if (resolvedFunc) {
                        tableIdx = this->addFunctionToTable(resolvedFunc);
                        globalOffsetTableMap.insert({name, tableIdx});
                    }
                }

                // If not found, create a placeholder to be filled in later
                // TODO - what causes this?
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
                              asString(getExternType(resolved)).c_str(),
                              asString(type).c_str());
                return false;
            }
        }

        logger->error("Missing import {}.{} {}", moduleName, name, asString(type).c_str());

        return false;
    }

    std::map<std::string, std::string> WasmModule::buildDisassemblyMap() {
        std::map<std::string, std::string> output;

        IRModuleRegistry &moduleRegistry = wasm::getIRModuleRegistry();
        IR::Module &module = moduleRegistry.getModule(this->boundUser, this->boundFunction, "");

        IR::DisassemblyNames disassemblyNames;
        getDisassemblyNames(module, disassemblyNames);

        for (Uptr i = 0; i < module.functions.size(); i++) {
            unsigned long nImports = module.functions.imports.size();
            bool isImport = i < nImports;

            int nameIdx = isImport ? i : i - nImports;
            std::string baseName = isImport ? "functionImport" : "functionDef";
            std::string funcName = baseName + std::to_string(nameIdx);

            std::string disasName = disassemblyNames.functions[i].name;
            output.insert({funcName, disasName});
        }

        return output;
    }

    void WasmModule::addFdForThisThread(int fd) {
        openFds.insert(fd);
    }

    void WasmModule::removeFdForThisThread(int fd) {
        openFds.erase(fd);
    }

    void WasmModule::clearFds() {
        openFds.clear();
    }

    void WasmModule::checkThreadOwnsFd(int fd) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        bool isNotOwned = openFds.find(fd) == openFds.end();
        util::SystemConfig &conf = util::getSystemConfig();

        if (fd == STDIN_FILENO) {
            if (conf.unsafeMode == "on") {
                logger->warn("Process interacting with stdin");
            } else {
                logger->error("Process interacting with stdin");
                throw std::runtime_error("Process interacting with stdin");
            }
        } else if (fd == STDOUT_FILENO) {
            // Can allow stdout/ stderr through
            // logger->debug("Process interacting with stdout", fd);
        } else if (fd == STDERR_FILENO) {
            // logger->debug("Process interacting with stderr", fd);
        } else if (isNotOwned) {
            logger->error("fd not owned by this thread {}", fd);
            throw std::runtime_error("fd not owned by this function");
        }
    }

    std::string WasmModule::getBoundUser() {
        return boundUser;
    }

    std::string WasmModule::getBoundFunction() {
        return boundFunction;
    }

    WasmModuleRegistry &getWasmModuleRegistry() {
        static WasmModuleRegistry w;
        return w;
    }

    WasmModuleRegistry::WasmModuleRegistry() = default;

    void WasmModuleRegistry::registerModule(const std::string &key, const WasmModule &module) {
        util::UniqueLock lock(registryMutex);

        // Be careful that we create a new instance here
        moduleMap.emplace(key, module);
    }

    WasmModule &WasmModuleRegistry::getModule(const std::string &key) {
        util::UniqueLock lock(registryMutex);

        if(moduleMap.count(key) == 0) {
            throw std::runtime_error("Key does not exist: " + key);
        }

        return moduleMap[key];
    }
}