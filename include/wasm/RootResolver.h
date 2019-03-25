#pragma once

#include <util/logging.h>

#include <string>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Inline/Serialization.h>
#include <WAVM/IR/Validate.h>

using namespace WAVM;

namespace wasm {

    // ------- WASM MEMORY CONSTANTS --------
    //
    // wasm memory is laid out as follows:
    //
    // |  | SP |   DATA   |   STACK   |   DYNAMIC   |   HEAP   brk ->   |
    // 0  4    5       data_end    heap_base      initial              MAX
    //
    // SP = stack pointer lives at address 4 as pre-defined in the spec
    // DATA = this holds any data segments
    // STACK = the stack (grows downwards)
    // DYNAMIC = fixed size empty region, can be used to create stuff on the fly (e.g. when dynamic linking)
    // HEAP = the heap (grows upwards), the heap will grow from the edge of your initial memory boundary and up
    //
    // __data_end and __heap_base should be exported from the main module and will be determined dynamically.
    // The initial memory size and max are determined when instantiating the module below.
    //
    // Note that page size in wasm is 64kiB
    // Note also that this initial memory must be big enough to include all data, stack and dynamic
    // memory that the module will need.
    const int ONE_MB_PAGES = 16;
    const int ONE_GB_PAGES = 1024 * ONE_MB_PAGES;

    const int INITIAL_MEMORY_PAGES = 30 * ONE_MB_PAGES;
    const int MAX_MEMORY_PAGES = ONE_GB_PAGES;

    // Emscripten memory constants
    const int EMSCRIPTEN_MIN_TABLE_ELEMS = 200000;
    const int EMSCRIPTEN_MAX_TABLE_ELEMS = 600000;
    const int INITIAL_EMSCRIPTEN_PAGES = 1024 * ONE_MB_PAGES;
    const int MAX_EMSCRIPTEN_PAGES = 2048 * ONE_GB_PAGES;
    const int EMSCRIPTEN_STACKTOP = 64 * IR::numBytesPerPage;
    const int EMSCRIPTEN_STACK_MAX = 256 * IR::numBytesPerPage;

    struct EmscriptenMutableGlobals {
        enum {
            address = 63 * IR::numBytesPerPage
        };

        U32 DYNAMICTOP_PTR;
        F64 tempDoublePtr;
        I32 _stderr;
        I32 _stdin;
        I32 _stdout;
    };

    enum class ioStreamVMHandle {
        StdErr = 1,
        StdIn = 2,
        StdOut = 3
    };

    // Standard toolchain version
    extern Intrinsics::Module &getIntrinsicModule_env();

    // Emscripten versions
    extern Intrinsics::Module &getIntrinsicModule_emEnv();

    extern Intrinsics::Module &getIntrinsicModule_emAsm2wasm();

    extern Intrinsics::Module &getIntrinsicModule_emGlobal();

    struct RootResolver : Runtime::Resolver {
        explicit RootResolver(Runtime::Compartment *compartmentIn) {
            _isSetUp = false;
            mainModule = nullptr;
            compartment = compartmentIn;
        }

        void setUser(const std::string &userIn) {
            user = userIn;
        }

        bool isSetUp() {
            return _isSetUp;
        }

        void setMainModule(Runtime::ModuleInstance *mainModuleIn) {
            mainModule = mainModuleIn;
        }

        void setUp(Runtime::Compartment *compartment, IR::Module &module) {
            // TODO - need to work out how to distinguish between emscripten/ non-emscripten
            isEmscripten = false;
            this->setUpStandardToolchain(compartment, module);

            // isEmscripten = true;
            // this->setUpEmscripten(compartment, module);

            _isSetUp = true;
        }

        void setUpStandardToolchain(Runtime::Compartment *compartment, IR::Module &module) {
            // Set up memory
            module.memories.imports[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;
            module.memories.imports[0].type.size.max = (U64) MAX_MEMORY_PAGES;
            Runtime::Memory *memory = Runtime::createMemory(compartment, module.memories.imports[0].type, "env.memory");

            // Add table
            Runtime::Table *table = Runtime::createTable(compartment, module.tables.imports[0].type,
                                                         "env.__indirect_function_table");

            HashMap<std::string, Runtime::Object *> extraEnvExports = {
                    {"memory",                    Runtime::asObject(memory)},
                    {"__indirect_function_table", Runtime::asObject(table)},
            };

            envModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_env(), "env",
                                                      extraEnvExports);
        }

        void setUpEmscripten(Runtime::Compartment *compartment, IR::Module &module) {
            // Memory constraints
            module.memories.imports[0].type.size.min = (U64) INITIAL_EMSCRIPTEN_PAGES;
            module.memories.imports[0].type.size.max = (U64) MAX_EMSCRIPTEN_PAGES;

//            module.tables.imports[0].type.size.min = (U64) EMSCRIPTEN_MIN_TABLE_ELEMS;
            module.tables.imports[0].type.size.max = (U64) EMSCRIPTEN_MAX_TABLE_ELEMS;

            Runtime::Memory *memory = Runtime::createMemory(compartment, module.memories.imports[0].type, "env.memory");
            Runtime::Table *table = Runtime::createTable(compartment, module.tables.imports[0].type, "env.table");

            HashMap<std::string, Runtime::Object *> extraEnvExports = {
                    {"memory", Runtime::asObject(memory)},
                    {"table",  Runtime::asObject(table)},
            };

            emEnvModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emEnv(), "env",
                                                        extraEnvExports);

            emAsm2wasmModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emAsm2wasm(),
                                                             "emAsm2wasm");

            emGlobalModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emGlobal(), "emGlobal");

            // Note: this MUST be a reference
            EmscriptenMutableGlobals &mutableGlobals = Runtime::memoryRef<EmscriptenMutableGlobals>(memory,
                                                                                                    EmscriptenMutableGlobals::address);
            mutableGlobals.DYNAMICTOP_PTR = EMSCRIPTEN_STACK_MAX;
            mutableGlobals._stderr = (U32) ioStreamVMHandle::StdErr;
            mutableGlobals._stdin = (U32) ioStreamVMHandle::StdIn;
            mutableGlobals._stdout = (U32) ioStreamVMHandle::StdOut;
        }

        void cleanUp() {
            envModule = nullptr;

            emEnvModule = nullptr;
            emAsm2wasmModule = nullptr;
            emGlobalModule = nullptr;
        }

        bool resolve(const std::string &moduleName,
                     const std::string &exportName,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            
            bool isDynamicModule = mainModule != nullptr;
            
            if (isEmscripten) {
                bool isGlobalAccessor = exportName.rfind("g$_", 0) == 0;
                bool isAFunc = type.kind == IR::ExternKind::function;

                if (isGlobalAccessor && isAFunc) {
                    logger->debug("Stubbing global accessor import {}", exportName);
                    resolved = getStubObject(exportName, type);
                    return true;
                }

                // Emscripten modules can have imports from 3 modules
                if (moduleName == "env") {
                    resolved = getInstanceExport(emEnvModule, exportName);
                } else if (moduleName == "asm2wasm") {
                    resolved = getInstanceExport(emAsm2wasmModule, exportName);
                } else if (moduleName == "global" || moduleName == "global.Math") {
                    resolved = getInstanceExport(emGlobalModule, exportName);
                } else {
                    logger->error("Unrecognised module: {}", moduleName);
                }
            } else if(isDynamicModule) {
                // The special cases below are the globals that are crucial to getting the
                // dynamic linking to work. They tell the new module about its new memory region
                if(exportName == "__memory_base") {
                    Runtime::Global *newMemoryBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newMemoryBase, nextMemoryBase);
                    resolved = asObject(newMemoryBase);
                }
                else if(exportName == "__table_base") {
                    Runtime::Global *newTableBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newTableBase, nextTableBase);
                    resolved = asObject(newTableBase);
                } else if(exportName == "__stack_pointer") {
                    Runtime::Global *newStackPointer = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newStackPointer, nextStackPointer);
                    resolved = asObject(newStackPointer);
                }
                else {
                    // Look in normal env
                    resolved = getInstanceExport(envModule, exportName);

                    // If not resolved here, check on the main module
                    if (!resolved) {
                        resolved = getInstanceExport(mainModule, exportName);
                    }
                }
            } else {
                // At this point we're resolving for a main module, so look in the env
                resolved = getInstanceExport(envModule, exportName);
            }

            // Check whether the function has been resolved to the correct type
            if (resolved) {
                if (isA(resolved, type)) {
                    return true;
                } else {
                    logger->error("Resolved import {}.{} to a {}, but was expecting {}",
                                  moduleName.c_str(),
                                  exportName.c_str(),
                                  asString(getObjectType(resolved)).c_str(),
                                  asString(type).c_str());
                    return false;
                }
            }

            logger->error("Missing import {}.{} {}", moduleName, exportName, asString(type).c_str());

            return false;
        }

        Runtime::Object *getStubObject(const std::string &exportName, IR::ExternType type) const {
            // If the import couldn't be resolved, stub it in.
            switch (type.kind) {
                case IR::ExternKind::function: {
                    // Generate a function body that just uses the unreachable op to fault if called.
                    Serialization::ArrayOutputStream codeStream;
                    IR::OperatorEncoderStream encoder(codeStream);
                    encoder.unreachable();
                    encoder.end();

                    // Generate a module for the stub function.
                    IR::Module stubIRModule;
                    IR::DisassemblyNames stubModuleNames;
                    stubIRModule.types.push_back(asFunctionType(type));
                    stubIRModule.functions.defs.push_back({{0}, {}, std::move(codeStream.getBytes()), {}});
                    stubIRModule.exports.push_back({"importStub", IR::ExternKind::function, 0});
                    stubModuleNames.functions.push_back({"importStub: " + exportName, {}, {}});
                    IR::setDisassemblyNames(stubIRModule, stubModuleNames);
                    IR::validatePreCodeSections(stubIRModule);
                    IR::validatePostCodeSections(stubIRModule);

                    // Instantiate the module and return the stub function instance.
                    auto stubModule = Runtime::compileModule(stubIRModule);
                    auto stubModuleInstance = instantiateModule(compartment, stubModule, {}, "importStub");
                    return getInstanceExport(stubModuleInstance, "importStub");
                }
                case IR::ExternKind::memory: {
                    return asObject(
                            Runtime::createMemory(compartment, asMemoryType(type), std::string(exportName)));
                }
                case IR::ExternKind::table: {
                    return asObject(
                            Runtime::createTable(compartment, asTableType(type), std::string(exportName)));
                }
                case IR::ExternKind::global: {
                    return asObject(Runtime::createGlobal(compartment, asGlobalType(type)));
                }
                case IR::ExternKind::exceptionType: {
                    return asObject(
                            Runtime::createExceptionType(compartment, asExceptionType(type), "importStub"));
                }
                default:
                    Errors::unreachable();
            };
        }

        bool isEmscripten = false;

        int nextMemoryBase;
        int nextTableBase;
        int nextStackPointer;
    private:
        // Main module (not mastered here)
        Runtime::ModuleInstance *mainModule;

        Runtime::Compartment *compartment;

        // Module for non-Emscripten
        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        // Emscripten modules
        Runtime::GCPointer<Runtime::ModuleInstance> emEnvModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emAsm2wasmModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emGlobalModule;

        std::string user;
        bool _isSetUp;
    };

}