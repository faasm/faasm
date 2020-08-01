#include "WAVMWasmModule.h"

#include <boost/filesystem.hpp>
#include <cereal/archives/binary.hpp>
#include <sys/mman.h>
#include <sys/types.h>

#include <ir_cache/IRModuleCache.h>
#include <storage/SharedFiles.h>
#include <util/bytes.h>
#include <util/func.h>
#include <util/memory.h>
#include <util/timing.h>
#include <util/config.h>
#include <util/locks.h>
#include <wasm/serialisation.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/Platform/Memory.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <Runtime/RuntimePrivate.h>
#include <WASI/WASIPrivate.h>

#include <wavm/openmp/ThreadState.h>
#include <wavm/OMPThreadPool.h>

constexpr int THREAD_STACK_SIZE(2 * ONE_MB_BYTES);

using namespace WAVM;

namespace wasm {
    static thread_local WAVMWasmModule *executingModule;

    static Runtime::Instance *baseEnvModule = nullptr;
    static Runtime::Instance *baseWasiModule = nullptr;

    std::mutex baseModuleMx;

    static void instantiateBaseModules() {
        if (baseEnvModule != nullptr) {
            return;
        }

        util::UniqueLock lock(baseModuleMx);

        // Double check
        if (baseEnvModule != nullptr) {
            return;
        }

        // Set up the basic modules common to all functions
        Runtime::Compartment *compartment = Runtime::createCompartment("baseModules");
        PROF_START(BaseEnvModule)
        baseEnvModule = Intrinsics::instantiateModule(
                compartment,
                {WAVM_INTRINSIC_MODULE_REF(env)},
                "env"
        );
        PROF_END(BaseEnvModule)

        PROF_START(BaseWasiModule)
        baseWasiModule = Intrinsics::instantiateModule(
                compartment,
                {WAVM_INTRINSIC_MODULE_REF(wasi)},
                "env"
        );
        PROF_END(BaseWasiModule)
    }

    Runtime::Instance *WAVMWasmModule::getEnvModule() {
        instantiateBaseModules();
        return baseEnvModule;
    }

    Runtime::Instance *WAVMWasmModule::getWasiModule() {
        instantiateBaseModules();
        return baseWasiModule;
    }

    WAVMWasmModule *getExecutingWAVMModule() {
        return executingModule;
    }

    void setExecutingModule(WAVMWasmModule *other) {
        executingModule = other;
    }

    WAVMWasmModule::WAVMWasmModule() {
        stdoutMemFd = 0;
        stdoutSize = 0;
    }

    WAVMWasmModule &WAVMWasmModule::operator=(const WAVMWasmModule &other) {
        PROF_START(wasmAssignOp)

        // Do the clone
        clone(other);

        PROF_END(wasmAssignOp)

        return *this;
    }

    WAVMWasmModule::WAVMWasmModule(const WAVMWasmModule &other) {
        PROF_START(wasmCopyConstruct)

        // Do the clone
        clone(other);

        PROF_END(wasmCopyConstruct)
    }

    void WAVMWasmModule::clone(const WAVMWasmModule &other) {
        // If bound, we want to reclaim all the memory we've created _before_ cloning from the zygote
        // otherwise it's lost forever
        if (_isBound) {
            tearDown();
        }

        dynamicModuleCount = other.dynamicModuleCount;
        nextMemoryBase = other.nextMemoryBase;
        nextStackPointer = other.nextStackPointer;
        nextTableBase = other.nextTableBase;

        memoryFd = other.memoryFd;
        memoryFdSize = other.memoryFdSize;

        _isBound = other._isBound;
        boundUser = other.boundUser;
        boundFunction = other.boundFunction;
        boundIsTypescript = other.boundIsTypescript;

        filesystem = other.filesystem;

        wasmEnvironment = other.wasmEnvironment;

        // Do not copy over any captured stdout
        stdoutMemFd = 0;
        stdoutSize = 0;

        if (other._isBound) {
            if (memoryFd > 0) {
                // Clone compartment excluding memory
                compartment = Runtime::cloneCompartment(other.compartment, "", false);
            } else {
                // Clone compartment including memory
                compartment = Runtime::cloneCompartment(other.compartment);
            }

            // Clone context
            executionContext = Runtime::cloneContext(other.executionContext, compartment);

            // Remap parts we need specific references to
            envModule = Runtime::remapToClonedCompartment(other.envModule, compartment);
            wasiModule = Runtime::remapToClonedCompartment(other.wasiModule, compartment);
            moduleInstance = Runtime::remapToClonedCompartment(other.moduleInstance, compartment);

            // Extract the memory and table again
            defaultMemory = Runtime::getDefaultMemory(moduleInstance);
            defaultTable = Runtime::getDefaultTable(moduleInstance);

            // Map memory contents if necessary
            if (memoryFd > 0) {
                mapMemoryFromFd();
            }

            // TODO - double check this works
            // Reset shared memory variables
            sharedMemWasmPtrs = other.sharedMemWasmPtrs;

            // Remap dynamic modules
            // TODO - double check this works
            dynamicPathToHandleMap = other.dynamicPathToHandleMap;
            for (auto &p : other.dynamicModuleMap) {
                Runtime::Instance *newInstance = Runtime::remapToClonedCompartment(p.second, compartment);
                dynamicModuleMap[p.first] = newInstance;
            }

            // Copy dynamic linking stuff
            globalOffsetTableMap = other.globalOffsetTableMap;
            globalOffsetMemoryMap = other.globalOffsetMemoryMap;
            missingGlobalOffsetEntries = other.missingGlobalOffsetEntries;
        }
    }

    WAVMWasmModule::~WAVMWasmModule() {
        tearDown();
    }

    bool WAVMWasmModule::tearDown() {
        PROF_START(wasmTearDown)

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // --- Faasm stuff ---
        sharedMemWasmPtrs.clear();

        globalOffsetTableMap.clear();
        globalOffsetMemoryMap.clear();
        missingGlobalOffsetEntries.clear();

        dynamicPathToHandleMap.clear();
        for (auto const &m : dynamicModuleMap) {
            dynamicModuleMap[m.first] = nullptr;
        }
        dynamicModuleMap.clear();

        // --- WAVM stuff ---

        // Set all reference to GC pointers to null to allow WAVM GC to clear up
        defaultMemory = nullptr;
        defaultTable = nullptr;
        moduleInstance = nullptr;

        envModule = nullptr;
        wasiModule = nullptr;

        executionContext = nullptr;

        if (compartment == nullptr) {
            return true;
        }

        bool compartmentCleared = Runtime::tryCollectCompartment(std::move(compartment));
        if (!compartmentCleared) {
            logger->debug("Failed GC for compartment");
        } else {
            logger->debug("Successful GC for compartment");
        }

        PROF_END(wasmTearDown)

        return compartmentCleared;
    }

    const bool WAVMWasmModule::isBound() {
        return _isBound;
    }

    Runtime::Function *
    WAVMWasmModule::getFunction(Runtime::Instance *module, const std::string &funcName, bool strict) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Look up the function
        Runtime::Function *func = asFunctionNullable(getInstanceExport(module, funcName));
        if (!func && strict) {
            logger->error("Unable to find function {}", funcName);
            throw std::runtime_error("Missing exported function");
        }

        return func;
    }

    void WAVMWasmModule::addModuleToGOT(IR::Module &mod, bool isMainModule) {
        // This function is **critical** for dynamic linking to work properly, but the underlying
        // spec is still in flux so it may break. The wasm dynamic linking docs can be found here:
        // https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md
        //
        // To handle dynamically loaded modules, we need to provide a "global offset table (GOT)" which
        // holds offsets to all the functions and data the modules may need to access.

        // Retrieve the disassembly names to help with building the GOT
        IR::DisassemblyNames disassemblyNames;
        getDisassemblyNames(mod, disassemblyNames);

        // If we add all table elements this map gets very large, therefore we just want
        // to include functions that the module explicitly exports.
        std::unordered_set<std::string> moduleExports;
        for(auto &e : mod.exports) {
            moduleExports.insert(e.name);
        }

        // ----------------------------
        // Table elems
        // ----------------------------
        // Here we need to inspect the module's table, and add all the entires along with
        // their offsets into our GOT.
        //
        // Here we iterate through the table definition from the module, working out the offsets
        // as we go.
        for (auto &es : mod.elemSegments) {
            // Work out the offset
            I32 offset;
            if (isMainModule) {
                // Main modules specify the offsets
                offset = es.baseOffset.i32;
            } else {
                // We control the base offset for dynamically loaded modules
                offset = nextTableBase;
            }

            // Go through each elem entry and record where in the table it's getting inserted
            for (size_t i = 0; i < es.contents->elemIndices.size(); i++) {
                unsigned long elemIdx = es.contents->elemIndices[i];
                // Work out the function's name, then add it to our GOT
                std::string &elemName = disassemblyNames.functions[elemIdx].name;

                if(moduleExports.find(elemName) != moduleExports.end()) {
                    Uptr tableIdx = offset + i;
                    globalOffsetTableMap.insert({elemName, tableIdx});
                }
            }
        }

        // ----------------------------
        // Data entries
        // ----------------------------
        // The data part of the GOT needs to map exports to their initialised value.
        // We only look at immutable i32s as this table seems to be mostly used for pointers.
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

    void WAVMWasmModule::executeFunction(
            Runtime::Function *func,
            IR::FunctionType funcType,
            const std::vector<IR::UntaggedValue> &arguments,
            IR::UntaggedValue &result
    ) {
        // Note the need to set the currently executing module
        setExecutingModule(this);

        // Function expects a result array so pass pointer to single value
        Runtime::invokeFunction(executionContext, func, funcType, arguments.data(), &result);
    }

    void WAVMWasmModule::bindToFunction(const message::Message &msg) {
        doBindToFunction(msg, true);
    }

    void WAVMWasmModule::bindToFunctionNoZygote(const message::Message &msg) {
        doBindToFunction(msg, false);
    }

    void WAVMWasmModule::doBindToFunction(const message::Message &msg, bool executeZygote) {
        /*
         * NOTE - the order things happen in this function is important.
         * The zygote function may execute non-trivial code and modify the memory,
         * but in order to work it needs the memory etc. to be set up.
         */

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        if (_isBound) {
            throw std::runtime_error("Cannot bind a module twice");
        }

        // Record that this module is now bound
        _isBound = true;
        boundIsTypescript = msg.istypescript();

        boundUser = msg.user();
        boundFunction = msg.function();

        // Set up the compartment and context
        PROF_START(wasmContext)
        compartment = Runtime::createCompartment();
        executionContext = Runtime::createContext(compartment);
        PROF_END(wasmContext)

        // Create the module instance
        moduleInstance = createModuleInstance(util::funcToString(msg, false), "");

        PROF_START(wasmBind)

        // Keep reference to memory and table
        defaultMemory = Runtime::getDefaultMemory(moduleInstance);
        defaultTable = Runtime::getDefaultTable(moduleInstance);

        // Prepare the filesystem
        filesystem.prepareFilesystem();

        // Execute the wasm ctors function. This is a hook generated by the linker
        // that lets things set up the environment (e.g. handling preopened
        // file descriptors).
        executeWasmConstructorsFunction(moduleInstance);

        // Get and execute zygote function
        if (executeZygote) {
            executeZygoteFunction();
        }

        // Typescript doesn't export memory info, nor does it require
        // setting up argv/argc
        if (!boundIsTypescript) {
            // Check stack is at the bottom
            I32 heapBase = getGlobalI32("__heap_base", executionContext);
            I32 dataEnd = getGlobalI32("__data_end", executionContext);

            if (heapBase > 0 && dataEnd > 0 && heapBase != dataEnd) {
                logger->error("Appears stack is not at bottom (__heap_base={} __data_end={})", heapBase, dataEnd);
                throw std::runtime_error("Wasm memory layout not as expected");
            }

            Uptr initialTableSize = Runtime::getTableNumElements(defaultTable);
            Uptr initialMemorySize = Runtime::getMemoryNumPages(defaultMemory) * WASM_BYTES_PER_PAGE;
            Uptr initialMemoryPages = Runtime::getMemoryNumPages(defaultMemory);

            logger->debug("heap_top={} initial_pages={} initial_table={}", initialMemorySize, initialMemoryPages,
                          initialTableSize);
        }

        PROF_END(wasmBind)
    }

    void
    WAVMWasmModule::writeStringArrayToMemory(const std::vector<std::string> &strings, U32 strPoitners, U32 strBuffer) {
        // Iterate through values, putting them in memory
        U32 strNextBuffer = strBuffer;
        U32 strNextPointer = strPoitners;

        for (const auto &thisStr : strings) {
            // Write this string to the buffer
            U8 *nextBuffer = &Runtime::memoryRef<U8>(defaultMemory, strNextBuffer);
            std::copy(thisStr.begin(), thisStr.end(), nextBuffer);

            // Write the pointer
            U32 *nextPointer = &Runtime::memoryRef<U32>(defaultMemory, strNextPointer);
            *nextPointer = strNextBuffer;

            // Move everything along , allowing space for a null terminator on the string
            strNextBuffer += thisStr.size() + 1;
            strNextPointer += sizeof(U32);
        }
    }

    void WAVMWasmModule::writeArgvToMemory(U32 wasmArgvPointers, U32 wasmArgvBuffer) {
        writeStringArrayToMemory(argv, wasmArgvPointers, wasmArgvBuffer);
    }

    void WAVMWasmModule::writeWasmEnvToMemory(U32 envPointers, U32 envBuffer) {
        writeStringArrayToMemory(wasmEnvironment.getVars(), envPointers, envBuffer);
    }

    Runtime::Instance *
    WAVMWasmModule::createModuleInstance(const std::string &name, const std::string &sharedModulePath) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        PROF_START(wasmCreateModule)

        IRModuleCache &moduleRegistry = wasm::getIRModuleCache();
        bool isMainModule = sharedModulePath.empty();

        // Warning: be very careful here to stick to *references* to the same shared modules
        // rather than creating copies.
        IR::Module &irModule = moduleRegistry.getModule(boundUser, boundFunction, sharedModulePath);

        if (isMainModule) {
            // Set up intrinsics
            if (boundIsTypescript) {
                // Special typescript env
                envModule = Intrinsics::instantiateModule(
                        compartment,
                        {WAVM_INTRINSIC_MODULE_REF(tsenv)},
                        "env"
                );
            } else {
                // Normal (C/C++) env
                envModule = Runtime::cloneInstance(getEnvModule(), compartment);

                // WASI
                wasiModule = Runtime::cloneInstance(getWasiModule(), compartment);
            }

            // Make sure the stack top is as expected
            IR::GlobalDef stackDef = irModule.globals.getDef(0);
            if (!stackDef.type.isMutable) {
                throw std::runtime_error("Found immutable stack top");
            }
        } else {
            // A dynamic module needs the same resources as a main module but we need to manually
            // create them

            // Give the module a chunk of memory and stack region just at the bottom of the
            // new memory (which will grow down). The memory sits above that (and grows up).
            // TODO - how do we detect stack overflows in dynamic modules? Are we meant to share the stack pointer of the main module?
            U32 dynamicMemBase = mmapPages(DYNAMIC_MODULE_HEAP_PAGES);
            nextMemoryBase = dynamicMemBase + DYNAMIC_MODULE_STACK_SIZE;
            nextStackPointer = nextMemoryBase - 1;

            // Extend the existing table to fit all the new elements from the dynamic module
            U64 nTableElems = moduleRegistry.getSharedModuleTableSize(boundUser, boundFunction,
                                                                      sharedModulePath);

            Uptr oldTableElems = 0;
            Runtime::GrowResult growResult = Runtime::growTable(defaultTable, nTableElems, &oldTableElems);
            if (growResult != Runtime::GrowResult::success) {
                throw std::runtime_error("Failed to grow main module table");
            }
            Uptr newTableElems = Runtime::getTableNumElements(defaultTable);

            // Set the base of the new module's table to be the top of the existing one.
            nextTableBase = oldTableElems;

            logger->debug("Provisioned new dynamic module region (stack_ptr={}, heap_base={}, table={}->{})",
                          nextStackPointer,
                          nextMemoryBase,
                          nextTableBase,
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

        Runtime::ModuleRef compiledModule = moduleRegistry.getCompiledModule(boundUser, boundFunction,
                                                                             sharedModulePath);

        logger->info("Instantiating module {}/{}  {}", boundUser, boundFunction, sharedModulePath);
        Runtime::Instance *instance = instantiateModule(
                compartment,
                compiledModule,
                std::move(linkResult.resolvedImports),
                name.c_str()
        );
        logger->info("Finished instantiating module {}/{}  {}", boundUser, boundFunction, sharedModulePath);

        // Here there may be some entries missing from the GOT that we need to patch up. They may
        // be exported from the dynamic module itself. I don't know how this happens but occasionally
        // it does
        if (!missingGlobalOffsetEntries.empty()) {
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

        PROF_END(wasmCreateModule)

        return instance;
    }

    I32 WAVMWasmModule::getGlobalI32(const std::string &globalName, Runtime::Context *context) {
        Runtime::Global *globalPtr = Runtime::asGlobal(Runtime::getInstanceExport(moduleInstance, globalName.c_str()));

        if (globalPtr == nullptr) {
            return -1;
        }

        const IR::Value &value = Runtime::getGlobalValue(context, globalPtr);
        return value.i32;
    }

    int WAVMWasmModule::dynamicLoadModule(const std::string &path, Runtime::Context *context) {
        // This function is essentially dlopen. See the comments around the GOT function
        // for more detail on the dynamic linking approach.

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
        Runtime::Instance *mod = createModuleInstance(name, path);

        // Execute wasm initialisers
        executeWasmConstructorsFunction(mod);

        // Keep a record of this module
        dynamicPathToHandleMap[path] = nextHandle;
        dynamicModuleMap[nextHandle] = mod;
        dynamicModuleCount++;

        logger->debug("Loaded shared module at {} with handle {}", path, nextHandle);

        return nextHandle;
    }

    uint32_t WAVMWasmModule::getDynamicModuleFunction(int handle, const std::string &funcName) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check the handle is valid
        if (dynamicModuleMap.count(handle) == 0) {
            logger->error("No dynamic module registered for handle {}", handle);
            throw std::runtime_error("Missing dynamic module");
        }

        // If function not in table for some reason, we need to load it
        Runtime::Instance *dynModule = dynamicModuleMap[handle];
        Runtime::Object *exportedFunc = getInstanceExport(dynModule, funcName);

        if (!exportedFunc) {
            logger->error("Unable to dynamically load function {}", funcName);
            throw std::runtime_error("Missing dynamic module function");
        }

        Uptr tableIdx = addFunctionToTable(exportedFunc);

        logger->debug("Resolved function {} to index {}", funcName, tableIdx);
        return tableIdx;
    }

    uint32_t WAVMWasmModule::addFunctionToTable(Runtime::Object *exportedFunc) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Add function to the table
        Uptr prevIdx;
        Runtime::GrowResult result = Runtime::growTable(defaultTable, 1, &prevIdx);
        if (result != Runtime::GrowResult::success) {
            logger->error("Failed to grow table from {} elements to {}", prevIdx, prevIdx + 1);
            throw std::runtime_error("Failed to grow table");
        }

        Uptr newElements = Runtime::getTableNumElements(defaultTable);
        logger->debug("Table grown from {} elements to {}", prevIdx, newElements);

        Runtime::setTableElement(defaultTable, prevIdx, exportedFunc);
        return prevIdx;
    }

    /**
     * Executes the given function call
     */
    bool WAVMWasmModule::execute(message::Message &msg, bool forceNoop) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!_isBound) {
            throw std::runtime_error("WasmModule must be bound before executing function");
        } else {
            if (boundUser != msg.user() || boundFunction != msg.function()) {
                const std::string funcStr = util::funcToString(msg, true);
                logger->error("Cannot execute {} on module bound to {}/{}",
                              funcStr, boundUser, boundFunction);
                throw std::runtime_error("Cannot execute function on module bound to another");
            }
        }

        setExecutingModule(this);
        setExecutingCall(&msg);

        // Ensure Python function file in place (if necessary)
        storage::SharedFiles::syncPythonFunctionFile(msg);

        // Set up OMP
        prepareOpenMPContext(msg);

        // Executes OMP fork message if necessary
        if (msg.ompdepth() > 0) {
            executeRemoteOMP(msg);
            return true;
        }

        // Run a specific function if requested
        int funcPtr = msg.funcptr();
        std::vector<IR::UntaggedValue> invokeArgs;
        Runtime::Function *funcInstance;
        IR::FunctionType funcType;

        if (funcPtr > 0) {
            // Get the function this call is referring to
            funcInstance = getFunctionFromPtr(funcPtr);

            // NOTE - when we've got a function pointer, we assume the args are a single integer
            // held in the input data (resulting from a chained thread invocation)
            if (msg.inputdata().empty()) {
                invokeArgs = {0};
            } else {
                int intArg = std::stoi(msg.inputdata());
                invokeArgs = {intArg};
            }

            funcType = IR::FunctionType(
                    {IR::ValueType::i32},
                    {IR::ValueType::i32}
            );
        } else if (boundIsTypescript) {
            // Different function signature for typescript
            funcType = IR::FunctionType(
                    {IR::ValueType::i32},
                    {}
            );
        } else {
            // Set up main args
            prepareArgcArgv(msg);

            // Get the entrypoint function (this is _start at the time of writing,
            // which is () -> void)
            funcInstance = getMainFunction(moduleInstance);
            funcType = IR::FunctionType(
                    {},
                    {}
            );
        }

        // Call the function
        int returnValue = 0;
        bool success = true;
        if (forceNoop) {
            logger->debug("NOTE: Explicitly forcing a noop");
        } else {

            try {
                Runtime::catchRuntimeExceptions([this, &funcInstance, &funcType, &invokeArgs, &returnValue, &logger] {
                    logger->debug("Invoking C/C++ function");

                    IR::UntaggedValue result;

                    executeFunction(
                            funcInstance,
                            funcType,
                            invokeArgs,
                            result
                    );

                    returnValue = result.i32;
                }, [&logger, &success, &returnValue](Runtime::Exception *ex) {
                    logger->error("Runtime exception: {}", Runtime::describeException(ex).c_str());
                    Runtime::destroyException(ex);
                    success = false;
                    returnValue = 1;
                });
            }
            catch (wasm::WasmExitException &e) {
                logger->debug("Caught wasm exit exception (code {})", e.exitCode);
                returnValue = e.exitCode;
                success = e.exitCode == 0;
            }
        }

        // Record the return value
        msg.set_returnvalue(returnValue);

        return success;
    }

    void WAVMWasmModule::executeRemoteOMP(message::Message &msg) {
        int funcPtr = msg.funcptr();
        std::vector<IR::UntaggedValue> invokeArgs;
        Runtime::Function *funcInstance;

        // Handle OMP functions
        funcInstance = getFunctionFromPtr(funcPtr);
        int threadNum = msg.ompthreadnum();
        int argc = msg.ompfunctionargs_size();

        util::getLogger()->debug("Running OMP thread #{} for function{} (argc = {})", threadNum, funcPtr, argc);

        invokeArgs.emplace_back(threadNum);
        invokeArgs.emplace_back(argc);
        for (int argIdx = argc - 1; argIdx >= 0; argIdx--) {
            invokeArgs.emplace_back(msg.ompfunctionargs(argIdx));
        }

        WasmThreadSpec spec = {
                getContextRuntimeData(executionContext),
                funcInstance,
                invokeArgs.data(),
                getExecutingWAVMModule()->allocateThreadStack(),
        };

        // Record the return value
        msg.set_returnvalue(executeThreadLocally(spec));
    }

    U32 WAVMWasmModule::mmapFile(U32 fd, U32 length) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // mmap the memory region
        U32 wasmPtr = mmapMemory(length);
        U32 *targetPtr = &Runtime::memoryRef<U32>(defaultMemory, wasmPtr);

        // Unmap and remap the memory
        munmap(targetPtr, length);
        U32 *mmappedPtr = (U32 *) mmap(targetPtr, length, PROT_READ, MAP_SHARED, fd, 0);
        if (mmappedPtr == MAP_FAILED) {
            logger->error("Failed mmapping file descriptor {} ({} - {})", fd, errno, strerror(errno));
            throw std::runtime_error("Unable to map file");
        }

        if (mmappedPtr != targetPtr) {
            throw std::runtime_error("Unable to map file into required location");
        }

        return wasmPtr;
    }

    U32 WAVMWasmModule::allocateThreadStack() {
        return this->mmapMemory(THREAD_STACK_SIZE);
    }

    U32 WAVMWasmModule::mmapMemory(U32 length) {
        // Round up to page boundary
        Uptr nWasmPages = getNumberOfWasmPagesForBytes(length);
        return mmapPages(nWasmPages);
    }

    U32 WAVMWasmModule::mmapPages(U32 pages) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        U64 maxSize = getMemoryType(defaultMemory).size.max;
        Uptr currentPageCount = Runtime::getMemoryNumPages(defaultMemory);

        if(pages == 0) {
            throw std::runtime_error("Requesting mapping of zero pages");
        }

        Uptr newPageCount = currentPageCount + pages;
        if (newPageCount > maxSize) {
            logger->error("mmap would exceed max of {} pages (growing by {} from {})", maxSize, pages,
                          currentPageCount);
            throw std::runtime_error("Mmap exceeding max");
        }

        Uptr pageCountOut;
        Runtime::GrowResult result = growMemory(defaultMemory, pages, &pageCountOut);
        if (result != Runtime::GrowResult::success) {
            if (result == Runtime::GrowResult::outOfMemory) {
                logger->error("Committing new pages failed (errno={} ({})) (growing by {} from current {})",
                              errno, strerror(errno), pages, currentPageCount);
                throw std::runtime_error("Unable to commit virtual pages");
            } else if (result == Runtime::GrowResult::outOfMaxSize) {
                logger->error("No memory for mapping (growing by {} from {} pages)", pages, currentPageCount);
                throw std::runtime_error("Run out of memory to map");
            } else if (result == Runtime::GrowResult::outOfQuota) {
                logger->error("Memory resource quota exceeded (growing by {} from {})", pages, newPageCount);
                throw std::runtime_error("Memory resource quota exceeded");
            } else {
                logger->error("Unknown memory mapping error (growing by {} from {}. Previous {})", pages, newPageCount,
                              pageCountOut);
                throw std::runtime_error("Unknown memory mapping error");
            }
        }

        logger->debug("mmap - Growing memory from {} to {} pages", currentPageCount, newPageCount);

        // Get pointer to mapped range
        auto mappedRangePtr = (U32) (Uptr(pageCountOut) * WASM_BYTES_PER_PAGE);

        return mappedRangePtr;
    }

    uint8_t* WAVMWasmModule::wasmPointerToNative(int32_t wasmPtr) {
        auto wasmMemoryRegionPtr = &Runtime::memoryRef<U8>(defaultMemory, wasmPtr);
        return wasmMemoryRegionPtr;
    }

    bool WAVMWasmModule::resolve(const std::string &moduleName,
                                 const std::string &name,
                                 IR::ExternType type,
                                 Runtime::Object *&resolved) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        bool isMainModule = moduleInstance == nullptr;

        Runtime::Instance *modulePtr = nullptr;
        if (moduleName == "wasi_snapshot_preview1") {
            modulePtr = wasiModule;
        } else {
            // Default to env module
            modulePtr = envModule;
        }

        if (isMainModule) {
            // Main module linking comes from env module or WASI
            resolved = getInstanceExport(modulePtr, name);
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
                Runtime::Global *gotMemoryOffset = Runtime::createGlobal(compartment, asGlobalType(type),
                                                                         std::string(name));
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
                        tableIdx = addFunctionToTable(resolvedFunc);
                        globalOffsetTableMap.insert({name, tableIdx});
                    }
                }

                // If not found, create a placeholder to be filled in later
                // TODO - what causes this?
                if (tableIdx == -1) {
                    // Create a new entry in the table and use this, but mark it to be filled later
                    Uptr newIdx;
                    Runtime::GrowResult result = Runtime::growTable(defaultTable, 1, &newIdx);

                    if (result != Runtime::GrowResult::success) {
                        throw std::runtime_error("Failed to grow table");
                    }

                    tableIdx = (int) newIdx;

                    logger->warn("Adding placeholder table offset: {}.{} at {}", moduleName, name, tableIdx);
                    missingGlobalOffsetEntries.insert({name, tableIdx});
                }

                // Create a global to hold the function offset
                Runtime::Global *gotFunctionOffset = Runtime::createGlobal(compartment, asGlobalType(type),
                                                                           std::string(name));
                Runtime::initializeGlobal(gotFunctionOffset, tableIdx);
                resolved = asObject(gotFunctionOffset);

            } else if (name == "__memory_base") {
                // Memory base tells the loaded module where to start its heap
                Runtime::Global *newMemoryBase = Runtime::createGlobal(compartment, asGlobalType(type),
                                                                       std::string(name));
                Runtime::initializeGlobal(newMemoryBase, nextMemoryBase);
                resolved = asObject(newMemoryBase);

            } else if (name == "__table_base") {
                // Table base tells the loaded module where to start its table entries
                Runtime::Global *newTableBase = Runtime::createGlobal(compartment, asGlobalType(type),
                                                                      std::string(name));
                Runtime::initializeGlobal(newTableBase, nextTableBase);
                resolved = asObject(newTableBase);

            } else if (name == "__stack_pointer") {
                // Stack pointer is where the loaded module should put its stack
                Runtime::Global *newStackPointer = Runtime::createGlobal(compartment, asGlobalType(type),
                                                                         std::string(name));
                Runtime::initializeGlobal(newStackPointer, nextStackPointer);
                resolved = asObject(newStackPointer);

            } else if (name == "__indirect_function_table") {
                // This is the name for the table imported from the main module
                Runtime::Table *table = Runtime::getDefaultTable(moduleInstance);
                resolved = asObject(table);
            } else {
                // First check in normal env
                resolved = getInstanceExport(modulePtr, name);

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

    std::map<std::string, std::string> WAVMWasmModule::buildDisassemblyMap() {
        std::map<std::string, std::string> output;

        IRModuleCache &moduleRegistry = wasm::getIRModuleCache();
        IR::Module &module = moduleRegistry.getModule(boundUser, boundFunction, "");

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

    int WAVMWasmModule::getDynamicModuleCount() {
        return dynamicModuleCount;
    }

    int WAVMWasmModule::getNextMemoryBase() {
        return nextMemoryBase;
    }

    int WAVMWasmModule::getNextStackPointer() {
        return nextStackPointer;
    }

    int WAVMWasmModule::getNextTableBase() {
        return nextTableBase;
    }

    int WAVMWasmModule::getFunctionOffsetFromGOT(const std::string &funcName) {
        if (globalOffsetTableMap.count(funcName) == 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Function not found in GOT - {}", funcName);
            throw std::runtime_error("Function not found in GOT");
        }

        return globalOffsetTableMap[funcName];
    }

    int WAVMWasmModule::getDataOffsetFromGOT(const std::string &name) {
        if (globalOffsetMemoryMap.count(name) == 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Data not found in GOT - {}", name);
            throw std::runtime_error("Memory not found in GOT");
        }

        return globalOffsetMemoryMap[name];
    }

    void WAVMWasmModule::writeMemoryToFd(int fd) {
        memoryFd = fd;

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Writing memory for {}/{} to fd {}", this->boundUser, this->boundFunction, memoryFd);

        Uptr numPages = Runtime::getMemoryNumPages(defaultMemory);
        Uptr numBytes = numPages * WASM_BYTES_PER_PAGE;
        U8 *memoryBase = Runtime::getMemoryBaseAddress(defaultMemory);

        // Make the fd big enough
        memoryFdSize = numBytes;
        int ferror = ftruncate(memoryFd, memoryFdSize);
        if (ferror) {
            logger->error("ferror call failed with error {}", ferror);
        }

        // Write the data
        ssize_t werror = write(memoryFd, memoryBase, memoryFdSize);
        if (werror == -1) {
            logger->error("write call failed");
        }
    }

    void WAVMWasmModule::mapMemoryFromFd() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Mapping memory for {}/{} from fd {}", this->boundUser, this->boundFunction, memoryFd);

        U8 *memoryBase = Runtime::getMemoryBaseAddress(defaultMemory);

        mmap(memoryBase, memoryFdSize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, memoryFd, 0);
    }

    void WAVMWasmModule::doSnapshot(std::ostream &outStream) {
        cereal::BinaryOutputArchive archive(outStream);

        // Serialise memory
        Uptr numPages = Runtime::getMemoryNumPages(defaultMemory);
        U8 *memBase = Runtime::getMemoryBaseAddress(defaultMemory);
        U8 *memEnd = memBase + (numPages * WASM_BYTES_PER_PAGE);

        wasm::MemorySerialised mem;
        mem.numPages = numPages;
        mem.data = std::vector<uint8_t>(memBase, memEnd);

        // Serialise to file
        archive(mem);
    }

    void WAVMWasmModule::doRestore(std::istream &inStream) {
        cereal::BinaryInputArchive archive(inStream);

        // Read in serialised data
        wasm::MemorySerialised mem;
        archive(mem);

        // Restore memory
        Uptr currentNumPages = Runtime::getMemoryNumPages(defaultMemory);
        size_t pagesRequired = mem.numPages - currentNumPages;
        if (pagesRequired > 0) {
            mmapPages(pagesRequired);
        }

        U8 *memBase = Runtime::getMemoryBaseAddress(defaultMemory);
        size_t memSize = mem.numPages * WASM_BYTES_PER_PAGE;
        memcpy(memBase, mem.data.data(), memSize);
    }


    /*
     * Creates a thread execution context
     * Assumes the worker module TLS was set up already
     */
    I64 WAVMWasmModule::executeThreadLocally(WasmThreadSpec &spec) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        // Create a new region for this thread's stack
        U32 thisStackBase = spec.stackTop;
        U32 stackTop = thisStackBase + THREAD_STACK_SIZE - 1;

        // Create a new context for this thread
        Runtime::Context *threadContext = createContext(
                getCompartmentFromContextRuntimeData(spec.contextRuntimeData)
        );

        // Set the stack pointer in this context
        IR::UntaggedValue &stackGlobal = threadContext->runtimeData->mutableGlobals[0];
        if (stackGlobal.u32 != STACK_SIZE) {
            util::getLogger()->error("Expected first mutable global in context to be stack pointer ({})",
                                     stackGlobal.u32);
            throw std::runtime_error("Unexpected mutable global format");
        }

        threadContext->runtimeData->mutableGlobals[0] = stackTop;

        int returnValue = 0;
        IR::UntaggedValue result;
        try {
            Runtime::catchRuntimeExceptions(
                    [this, &spec, &returnValue, &logger, &threadContext, &result] {
                        logger->debug("Invoking C/C++ function");

                        setExecutingModule(this);

                        // Execute the function
                        Runtime::invokeFunction(
                                threadContext,
                                spec.func,
                                Runtime::getFunctionType(spec.func),
                                spec.funcArgs,
                                &result
                        );

                        returnValue = result.i32;
                    }, [&logger, &returnValue](Runtime::Exception *ex) {
                        logger->error("Runtime exception: {}", Runtime::describeException(ex).c_str());
                        Runtime::destroyException(ex);
                        returnValue = 1;
                    });
        }
        catch (wasm::WasmExitException &e) {
            logger->debug("Caught wasm exit exception (code {})", e.exitCode);
            returnValue = e.exitCode;
        }

        return returnValue;
    }

    Runtime::Function *WAVMWasmModule::getMainFunction(Runtime::Instance *module) {
        std::string mainFuncName(ENTRY_FUNC_NAME);

        if (boundIsTypescript) {
            mainFuncName = "_asMain";
        }

        return getFunction(module, mainFuncName, true);
    }

    Runtime::Function *WAVMWasmModule::getWasmConstructorsFunction(Runtime::Instance *module) {
        return getFunction(module, WASM_CTORS_FUNC_NAME, false);
    }

    Runtime::Function *WAVMWasmModule::getDefaultZygoteFunction(Runtime::Instance *module) {
        return getFunction(module, ZYGOTE_FUNC_NAME, false);
    }

    void WAVMWasmModule::executeZygoteFunction() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        Runtime::Function *zygoteFunc = getDefaultZygoteFunction(moduleInstance);
        if (zygoteFunc) {
            IR::UntaggedValue result;
            const IR::FunctionType funcType = Runtime::getFunctionType(zygoteFunc);
            executeFunction(
                    zygoteFunc,
                    funcType,
                    {},
                    result
            );

            if (result.i32 != 0) {
                logger->error("Zygote for {}/{} failed with return code {}", boundUser, boundFunction, result.i32);
                throw std::runtime_error("Zygote failed");
            } else {
                logger->debug("Successfully executed zygote for {}/{}", boundUser, boundFunction);
            }
        }
    }

    void WAVMWasmModule::executeWasmConstructorsFunction(Runtime::Instance *module) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        Runtime::Function *wasmCtorsFunction = getWasmConstructorsFunction(module);
        if (!wasmCtorsFunction) {
            logger->error("Did not find __wasm_call_ctors function for {}/{}", boundUser, boundFunction);
            throw std::runtime_error("Did not find __wasm_call_ctors");
        }

        IR::UntaggedValue result;
        executeFunction(
                wasmCtorsFunction,
                IR::FunctionType({}, {}),
                {},
                result
        );

        if (result.i32 != 0) {
            logger->error("{} for {}/{} failed with return code {}",
                          WASM_CTORS_FUNC_NAME, boundUser, boundFunction, result.i32);
            throw std::runtime_error(std::string(WASM_CTORS_FUNC_NAME) + " failed");
        } else {
            logger->debug("Successfully executed {} for {}/{}", WASM_CTORS_FUNC_NAME, boundUser, boundFunction);
        }
    }

    Runtime::Function *WAVMWasmModule::getFunctionFromPtr(int funcPtr) {
        Runtime::Object *funcObj = Runtime::getTableElement(defaultTable, funcPtr);
        return Runtime::asFunction(funcObj);
    }

    void WAVMWasmModule::prepareOpenMPContext(const message::Message &msg) {
        std::shared_ptr<openmp::Level> ompLevel;

        if (msg.ompdepth() > 0) {
            ompLevel = std::static_pointer_cast<openmp::Level>(
                    std::make_shared<openmp::MultiHostSumLevel>(msg.ompdepth(),
                                                                msg.ompeffdepth(),
                                                                msg.ompmal(),
                                                                msg.ompnumthreads()));
        } else {
            OMPPool = std::make_unique<openmp::PlatformThreadPool>(util::getSystemConfig().ompThreadPoolSize, this);
            ompLevel = std::static_pointer_cast<openmp::Level>(
                    std::make_shared<openmp::SingleHostLevel>());
        }

        openmp::setTLS(msg.ompthreadnum(), ompLevel);
    }

    std::unique_ptr<openmp::PlatformThreadPool> &WAVMWasmModule::getOMPPool() {
        return OMPPool;
    }
}
