#pragma once

#include <util/logging.h>

#include <string>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {

    // Note that the max memory per module is 8GiB, i.e. > 100k pages
    // Page size in wasm is 64kiB so 50 pages ~ 3MiB of memory
    const int INITIAL_MEMORY_PAGES = 100;
    const size_t INITIAL_MEMORY_SIZE = INITIAL_MEMORY_PAGES * IR::numBytesPerPage;

    enum {
        minStaticEmscriptenMemoryPages = 128
    };

    enum ErrNo {
        einval = 22
    };

    struct MutableGlobals {
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
        explicit RootResolver(Runtime::Compartment *compartment) {

        }

        void setUser(const std::string &userIn) {
            user = userIn;
        }

        void setUp(Runtime::Compartment *compartment, IR::Module &module) {
            // Detect if this is an emscripten function
            if (!module.memories.defs.empty()) {
                isEmscripten = false;
                this->setUpStandardToolchain(compartment, module);
            } else {
                isEmscripten = true;

                // TODO make this check better. Is it always a reliable way to detect Emscripten funcs?
                this->setUpEmscripten(compartment, module);
            }
        }

        void setUpStandardToolchain(Runtime::Compartment *compartment, IR::Module &module) {
            // Set up minimum memory size
            module.memories.defs[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;

            envModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_env(), "env");
        }

        void setUpEmscripten(Runtime::Compartment *compartment, IR::Module &module) {
            // Min memory pages
            module.memories.imports[0].type.size.min = (U64) 100;

            IR::TableType tableType(IR::ReferenceType::funcref, false, IR::SizeConstraints{0, 0});
            tableType = module.tables.imports[0].type;

            Runtime::Memory *memory = Runtime::createMemory(compartment, module.memories.imports[0].type, "env.memory");
            Runtime::Table *table = Runtime::createTable(compartment, tableType, "env.table");

            HashMap<std::string, Runtime::Object *> extraEnvExports = {
                    {"memory", Runtime::asObject(memory)},
                    {"table",  Runtime::asObject(table)},
            };

            emEnvModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emEnv(), "env",
                                                        extraEnvExports);

            emAsm2wasmModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emAsm2wasm(),
                                                             "emAsm2wasm");

            emGlobalModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emGlobal(), "emGlobal");

            MutableGlobals &mutableGlobals = Runtime::memoryRef<MutableGlobals>(memory, MutableGlobals::address);

            mutableGlobals.DYNAMICTOP_PTR = 128 * IR::numBytesPerPage;
            mutableGlobals._stderr = (U32) ioStreamVMHandle::StdErr;
            mutableGlobals._stdin = (U32) ioStreamVMHandle::StdIn;
            mutableGlobals._stdout = (U32) ioStreamVMHandle::StdOut;
        }

        void cleanUp() {
            envModule = nullptr;
            emAsm2wasmModule = nullptr;
            emGlobalModule = nullptr;
        }

        bool resolve(const std::string &moduleName,
                     const std::string &exportName,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            if (isEmscripten) {
                // Emscripten modules can have imports from 3 modules
                if (moduleName == "env") {
                    resolved = getInstanceExport(emEnvModule, exportName);
                } else if (moduleName == "asm2wasm") {
                    resolved = getInstanceExport(emAsm2wasmModule, exportName);
                } else if (moduleName == "global") {
                    resolved = getInstanceExport(emGlobalModule, exportName);
                } else {
                    logger->error("Unrecognised module: {}", moduleName);
                }
            } else {
                // In non-emscripten environments we only care about the env module
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

        bool isEmscripten = false;
    private:
        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        // Emscripten modules
        Runtime::GCPointer<Runtime::ModuleInstance> emEnvModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emAsm2wasmModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emGlobalModule;

        std::string user;
    };
}