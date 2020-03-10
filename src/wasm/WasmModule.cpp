#include "WasmModule.h"

#include <sys/mman.h>
#include <fstream>

#include <cereal/archives/binary.hpp>

#include <util/func.h>
#include <util/memory.h>
#include <util/timing.h>
#include <util/config.h>
#include <util/locks.h>

#include <wasm/IRModuleRegistry.h>
#include <wasm/serialisation.h>

#include <WAVM/WASM/WASM.h>
#include <WAVM/IR/Types.h>
#include <WAVM/Platform/Memory.h>
#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <Runtime/RuntimePrivate.h>
#include <WAVM/WASTParse/WASTParse.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/uio.h>

using namespace WAVM;

namespace wasm {
    // Using TLS here to isolate between executing functions
    static thread_local WasmModule *executingModule;
    static thread_local message::Message *executingCall;
    static thread_local std::set<int> openFds;

    WasmModule *getExecutingModule() {
        return executingModule;
    }

    void setExecutingModule(WasmModule *other) {
        executingModule = other;
    }

    message::Message *getExecutingCall() {
        return executingCall;
    }

    void setExecutingCall(message::Message *other) {
        executingCall = other;
    }

    Uptr getNumberOfPagesForBytes(U32 nBytes) {
        // Round up to nearest page
        Uptr pageCount = (Uptr(nBytes) + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        return pageCount;
    }

    WasmModule::WasmModule() : stdoutMemFd(0), stdoutSize(0) {

    }

    WasmModule &WasmModule::operator=(const WasmModule &other) {
        PROF_START(wasmAssignOp)

        // Do the clone
        clone(other);

        PROF_END(wasmAssignOp)

        return *this;
    }

    WasmModule::WasmModule(const WasmModule &other) {
        PROF_START(wasmCopyConstruct)

        // Do the clone
        clone(other);

        PROF_END(wasmCopyConstruct)
    }

    void WasmModule::clone(const WasmModule &other) {
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
            moduleInstance = Runtime::remapToClonedCompartment(other.moduleInstance, compartment);

            // NOTE: although we are using the remapping functions here, the function
            // variables are just pointers (and are returned directly)
            functionInstance = Runtime::remapToClonedCompartment(other.functionInstance, compartment);
            zygoteFunctionInstance = Runtime::remapToClonedCompartment(other.zygoteFunctionInstance, compartment);

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

    bool WasmModule::tearDown() {
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

        functionInstance = nullptr;
        zygoteFunctionInstance = nullptr;

        envModule = nullptr;

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


    WasmModule::~WasmModule() {
        tearDown();
    }

    const bool WasmModule::isBound() {
        return _isBound;
    }

    Runtime::Function *WasmModule::getFunction(const std::string &funcName, bool strict) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Look up the function
        Runtime::Function *func = asFunctionNullable(getInstanceExport(moduleInstance, funcName));
        if (!func && strict) {
            logger->error("Unable to find function {}", funcName);
            throw std::runtime_error("Missing exported function");
        }

        return func;
    }

    void WasmModule::addModuleToGOT(IR::Module &mod, bool isMainModule) {
        // This function is **critical** for dynamic linking to work properly, but the underlying
        // spec is still in flux so it may break. The wasm dynamic linking docs can be found here:
        // https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md
        //
        // To handle dynamically loaded modules, we need to provide a "global offset table (GOT)" which
        // holds offsets to all the functions and data the modules may need to access.

        // Retrieve the disassembly names to help with building the GOT
        IR::DisassemblyNames disassemblyNames;
        getDisassemblyNames(mod, disassemblyNames);

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
                Uptr tableIdx = offset + i;
                globalOffsetTableMap.insert({elemName, tableIdx});
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

    void WasmModule::executeFunction(
            Runtime::Function *func,
            IR::FunctionType funcType,
            const std::vector<IR::UntaggedValue> &arguments,
            IR::UntaggedValue &result
    ) {
        // Note the need to set the currently executing module
        executingModule = this;

        // Function expects a result array so pass pointer to single value
        Runtime::invokeFunction(executionContext, func, funcType, arguments.data(), &result);
    }

    void WasmModule::bindToFunction(const message::Message &msg, bool executeZygote) {
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

        // Get main entrypoint function
        std::string mainFuncName(ENTRY_FUNC_NAME);
        if (boundIsTypescript) {
            mainFuncName = "_asMain";
        }
        functionInstance = getFunction(mainFuncName, true);

        // Keep reference to memory and table
        defaultMemory = Runtime::getDefaultMemory(moduleInstance);
        defaultTable = Runtime::getDefaultTable(moduleInstance);

        // Get and execute zygote function
        if (executeZygote) {
            zygoteFunctionInstance = getFunction(ZYGOTE_FUNC_NAME, false);
            if (zygoteFunctionInstance) {
                IR::UntaggedValue result;
                const IR::FunctionType funcType = Runtime::getFunctionType(zygoteFunctionInstance);
                executeFunction(
                        zygoteFunctionInstance,
                        funcType,
                        {},
                        result
                );

                if (result.i32 != 0) {
                    logger->error("Zygote for {}/{} failed with return code {}", boundUser, boundFunction, result.i32);
                    throw std::runtime_error("Zygote failed");
                }
            }
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
            Uptr initialMemorySize = Runtime::getMemoryNumPages(defaultMemory) * IR::numBytesPerPage;
            Uptr initialMemoryPages = Runtime::getMemoryNumPages(defaultMemory);

            logger->debug("heap_top={} initial_pages={} initial_table={}", initialMemorySize, initialMemoryPages,
                          initialTableSize);
        }

        PROF_END(wasmBind)
    }

    std::vector<IR::UntaggedValue> WasmModule::getArgcArgv(const message::Message &msg) {
        if (msg.cmdline().empty()) {
            // Return argc=0 argv=NULL if no commandline args specified
            return {0, 0};
        }

        // Here we set up the arguments to main(), i.e. argc and argv
        // We allow passing of arbitrary commandline arguments via the invocation message.
        // These are passed as a string with a space separating each argument.
        const std::vector<std::string> argv = util::getArgvForMessage(msg);
        U32 argc = argv.size();
        size_t argvSize = argv.size() * sizeof(U32);

        // Create a new page for argv and its values to live in. At the start we put the array of
        // char pointers (i.e. the actual argv char* array), then above that we place the actual
        // strings.
        U32 argvPointersOffset = mmapMemory(1);
        U32 argvValuesOffset = argvPointersOffset + argvSize + 10;
        U32 *argvPointersHost = &Runtime::memoryRef<U32>(defaultMemory, argvPointersOffset);
        char *argvValuesHost = &Runtime::memoryRef<char>(defaultMemory, argvValuesOffset);

        for (uint i = 0; i < argv.size(); i++) {
            const std::string thisArgv = argv[i];

            // Write this string to memory and record its pointer
            std::copy(thisArgv.begin(), thisArgv.end(), argvValuesHost);
            argvPointersHost[i] = argvValuesOffset;

            // Move both pointers and values offset along
            // (allowing space for a null terminator on the string)
            argvValuesHost += thisArgv.size() + 1;
            argvValuesOffset += thisArgv.size() + 1;
        }

        // Create the vector with argc and the pointer to argv
        return {argc, argvPointersOffset};
    }

    Runtime::Instance *
    WasmModule::createModuleInstance(const std::string &name, const std::string &sharedModulePath) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        PROF_START(wasmCreateModule)
        IRModuleRegistry &moduleRegistry = wasm::getIRModuleRegistry();
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
                envModule = Intrinsics::instantiateModule(
                        compartment,
                        {WAVM_INTRINSIC_MODULE_REF(env)},
                        "env"
                );
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

    I32 WasmModule::getGlobalI32(const std::string &globalName, Runtime::Context *context) {
        Runtime::Global *globalPtr = Runtime::asGlobal(Runtime::getInstanceExport(moduleInstance, globalName.c_str()));

        if (globalPtr == nullptr) {
            return -1;
        }

        const IR::Value &value = Runtime::getGlobalValue(context, globalPtr);
        return value.i32;
    }

    int WasmModule::dynamicLoadModule(const std::string &path, Runtime::Context *context) {
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

        // Call the wasm constructor function. This allows the module to perform any relevant initialisation
        Runtime::Function *wasmCtorsFunction = asFunctionNullable(getInstanceExport(mod, "__wasm_call_ctors"));
        if (wasmCtorsFunction) {
            logger->debug("Executing __wasm_call_ctors for {}", path);
            IR::UntaggedValue result;
            executeFunction(
                    wasmCtorsFunction,
                    IR::FunctionType({}, {}),
                    {},
                    result
            );
        }

        // Keep a record of this module
        dynamicPathToHandleMap[path] = nextHandle;
        dynamicModuleMap[nextHandle] = mod;
        dynamicModuleCount++;

        logger->debug("Loaded shared module at {} with handle {}", path, nextHandle);

        return nextHandle;
    }

    Runtime::Instance *WasmModule::getDynamicModule(int handle) {
        if (dynamicModuleMap.count(handle) == 0) {
            throw std::runtime_error("Missing dynamic module");
        }

        Runtime::Instance *dynModule = dynamicModuleMap[handle];
        return dynModule;
    }

    Uptr WasmModule::getDynamicModuleFunction(int handle, const std::string &funcName) {
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

    Uptr WasmModule::addFunctionToTable(Runtime::Object *exportedFunc) {
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
    int WasmModule::execute(message::Message &msg) {
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

        executingModule = this;
        executingCall = &msg;

        int exitCode = 0;

        // Set up arguments
        std::vector<IR::UntaggedValue> invokeArgs = getArgcArgv(msg);

        try {
            // Call the function

            Runtime::catchRuntimeExceptions([this, &invokeArgs, &exitCode, &logger] {
                IR::UntaggedValue result;

                // Different function signature for different languages
                if (boundIsTypescript) {
                    logger->debug("Invoking typescript function");
                    executeFunction(
                            functionInstance,
                            IR::FunctionType(
                                    {IR::ValueType::i32},
                                    {}
                            ),
                            {},
                            result
                    );
                } else {
                    logger->debug("Invoking C/C++ function");

                    executeFunction(
                            functionInstance,
                            IR::FunctionType(
                                    {IR::ValueType::i32},
                                    {IR::ValueType::i32, IR::ValueType::i32}
                            ),
                            invokeArgs,
                            result
                    );
                }

                exitCode = result.i32;
            }, [&logger, &exitCode](Runtime::Exception *ex) {
                logger->error("Runtime exception: {}", Runtime::describeException(ex).c_str());
                Runtime::destroyException(ex);
                exitCode = 1;
            });
        }
        catch (wasm::WasmExitException &e) {
            exitCode = e.exitCode;
        }

        return exitCode;
    }

    U32 WasmModule::mmapFile(U32 fd, U32 length) {
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

    U32 WasmModule::mmapMemory(U32 length) {
        // Round up to page boundary
        Uptr pagesRequested = getNumberOfPagesForBytes(length);
        return mmapPages(pagesRequested);
    }

    U32 WasmModule::mmapPages(U32 pages) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        U64 maxSize = getMemoryType(defaultMemory).size.max;
        Uptr currentPageCount = Runtime::getMemoryNumPages(defaultMemory);

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
        auto mappedRangePtr = (U32) (Uptr(pageCountOut) * IR::numBytesPerPage);

        return mappedRangePtr;
    }

    U32 WasmModule::mmapKey(const std::shared_ptr<state::StateKeyValue> &kv, long offset, U32 length) {
        // Create a key for this specific offset and length and cache the pointer once done
        const std::string segmentKey = kv->getSegmentKey(offset, length);

        // See if this is the first time the module has seen this key
        if (sharedMemWasmPtrs.count(segmentKey) == 0) {
            size_t alignedOffset = util::alignOffsetDown(offset);
            size_t offsetFromStart = offset - alignedOffset;

            // Work out how many bytes are needed to contain the result once it's been page aligned
            size_t bytesLength = offsetFromStart + length;

            // Create new memory region that's big enough
            U32 wasmMemoryRegion = this->mmapMemory(bytesLength);
            U8 *hostMemPtr = &Runtime::memoryRef<U8>(defaultMemory, wasmMemoryRegion);

            // Map the WASM memory to the shared value
            void *voidPtr = static_cast<void *>(hostMemPtr);
            long offsetPages = util::getRequiredHostPagesRoundDown(offset);
            long nPages = util::getRequiredHostPages(bytesLength);
            kv->mapSharedMemory(voidPtr, offsetPages, nPages);

            // Remember the kv and pointer
            U32 wasmPtr = wasmMemoryRegion + offsetFromStart;
            sharedMemWasmPtrs.insert(std::pair<std::string, I32>(segmentKey, wasmPtr));
        }

        // Return the wasm pointer
        return sharedMemWasmPtrs[segmentKey];
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

    int WasmModule::getDynamicModuleCount() {
        return dynamicModuleCount;
    }

    int WasmModule::getNextMemoryBase() {
        return nextMemoryBase;
    }

    int WasmModule::getNextStackPointer() {
        return nextStackPointer;
    }

    int WasmModule::getNextTableBase() {
        return nextTableBase;
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

        if (fd == STDIN_FILENO) {
            logger->warn("Process interacting with stdin");
            return;
        } else if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
            // Allow stdout/ stderr
            return;
        }

        bool isNotOwned = openFds.find(fd) == openFds.end();
        if (isNotOwned) {
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

    bool WasmModule::getBoundIsTypescript() {
        return boundIsTypescript;
    }

    int WasmModule::getFunctionOffsetFromGOT(const std::string &funcName) {
        if (globalOffsetTableMap.count(funcName) == 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Function not found in GOT - {}", funcName);
            throw std::runtime_error("Function not found in GOT");
        }

        return globalOffsetTableMap[funcName];
    }

    int WasmModule::getDataOffsetFromGOT(const std::string &name) {
        if (globalOffsetMemoryMap.count(name) == 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Data not found in GOT - {}", name);
            throw std::runtime_error("Memory not found in GOT");
        }

        return globalOffsetMemoryMap[name];
    }

    void WasmModule::writeMemoryToFd(int fd) {
        memoryFd = fd;

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Writing memory for {}/{} to fd {}", this->boundUser, this->boundFunction, memoryFd);

        Uptr numPages = Runtime::getMemoryNumPages(defaultMemory);
        Uptr numBytes = numPages * IR::numBytesPerPage;
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

    void WasmModule::mapMemoryFromFd() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Mapping memory for {}/{} from fd {}", this->boundUser, this->boundFunction, memoryFd);

        U8 *memoryBase = Runtime::getMemoryBaseAddress(defaultMemory);

        mmap(memoryBase, memoryFdSize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, memoryFd, 0);
    }

    void WasmModule::snapshotCrossHost(const std::string &filePath) {
        std::ofstream outStream(filePath, std::ios::binary);
        cereal::BinaryOutputArchive archive(outStream);

        // Serialise memory
        Uptr numPages = Runtime::getMemoryNumPages(defaultMemory);
        U8 *memBase = Runtime::getMemoryBaseAddress(defaultMemory);
        U8 *memEnd = memBase + (numPages * IR::numBytesPerPage);

        wasm::MemorySerialised mem;
        mem.numPages = numPages;
        mem.data = std::vector<uint8_t>(memBase, memEnd);

        // Serialise to file
        archive(mem);
    }

    void WasmModule::restoreCrossHost(const message::Message &msg, const std::string &filePath) {
        // Read execution state from file
        std::ifstream inStream(filePath, std::ios::binary);
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
        size_t memSize = mem.numPages * IR::numBytesPerPage;
        memcpy(memBase, mem.data.data(), memSize);
    }

    int WasmModule::getStdoutFd() {
        if (stdoutMemFd == 0) {
            message::Message *call = getExecutingCall();
            const std::string fdName = std::to_string(call->id());
            stdoutMemFd = memfd_create(fdName.c_str(), 0);

            util::getLogger()->debug("Capturing stdout: fd={},{} for {}", fdName, stdoutMemFd,
                                     util::funcToString(*call, true));
        }

        return stdoutMemFd;
    }

    ssize_t WasmModule::captureStdout(const struct iovec *iovecs, int iovecCount) {
        int memFd = getStdoutFd();
        ssize_t writtenSize = writev(memFd, iovecs, iovecCount);

        if (writtenSize < 0) {
            util::getLogger()->error("Failed capturing stdout: {}", strerror(errno));
            throw std::runtime_error(std::string("Failed capturing stdout: ")
                                     + strerror(errno));
        }

        util::getLogger()->debug("Captured {} bytes of formatted stdout", writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    ssize_t WasmModule::captureStdout(const void *buffer) {
        int memFd = getStdoutFd();

        ssize_t writtenSize = dprintf(memFd, "%s\n", reinterpret_cast<const char *>(buffer));

        if (writtenSize < 0) {
            util::getLogger()->error("Failed capturing stdout: {}", strerror(errno));
            throw std::runtime_error("Failed capturing stdout");
        }

        util::getLogger()->debug("Captured {} bytes of unformatted stdout", writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    std::string WasmModule::getCapturedStdout() {
        if (stdoutSize == 0) {
            return "";
        }

        // Go back to start
        int memFd = getStdoutFd();
        lseek(memFd, 0, SEEK_SET);

        // Read in and return
        char *buf = new char[stdoutSize];
        read(memFd, buf, stdoutSize);
        std::string stdoutString(buf, stdoutSize);
        util::getLogger()->debug("Read stdout length {}:\n{}", stdoutSize, stdoutString);

        return stdoutString;
    }

    void WasmModule::clearCapturedStdout() {
        close(stdoutMemFd);
        stdoutMemFd = 0;
        stdoutSize = 0;
    }

    I64 WasmModule::executeThread(WasmThreadSpec &spec) {
        // Set up TLS for this thread
        setExecutingModule(spec.parentModule);
        setExecutingCall(spec.parentCall);

        // Create a new region for this thread's stack
        U32 thisStackBase = getExecutingModule()->mmapMemory(spec.stackSize);
        U32 stackTop = thisStackBase + spec.stackSize - 1;

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

        // Execute the function
        IR::UntaggedValue result;
        Runtime::invokeFunction(
                threadContext,
                spec.func,
                Runtime::getFunctionType(spec.func),
                spec.funcArgs,
                &result
        );

        return result.i32;
    }
}
