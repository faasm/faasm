#pragma once

#include <util/logging.h>

#include <string>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Inline/Serialization.h>
#include <WAVM/IR/Validate.h>
#include "../../WAVM/Lib/Runtime/RuntimePrivate.h"

using namespace WAVM;

namespace wasm {
    // Note that page size in wasm is 64kiB
    // Note also that this initial memory must be big enough to include all data, stack and dynamic
    // memory that the module will need.
    const int ONE_MB_PAGES = 16;
    const int ONE_GB_PAGES = 1024 * ONE_MB_PAGES;
    const int ONE_MB_BYTES = 1024 * 1024;

    // Stack size and max memory are hard-coded in the build
    const int STACK_SIZE = ONE_MB_BYTES;
    const int MAX_MEMORY_PAGES = ONE_GB_PAGES;

    // Properties of dynamic modules
    const int DYNAMIC_MODULE_STACK_SIZE = 1 * ONE_MB_PAGES;
    const int DYNAMIC_MODULE_HEAP_SIZE = 4 * ONE_MB_PAGES;
    const int DYNAMIC_MODULE_TABLE_ELEMS = 100;

    // const int INITIAL_MEMORY_PAGES = 1024 * ONE_MB_PAGES; // Hard-coded in build

    const int MAX_TABLE_SIZE = 500000;

    extern Intrinsics::Module &getIntrinsicModule_env();

    struct RootResolver : Runtime::Resolver {
        explicit RootResolver(Runtime::Compartment *compartmentIn) {
            mainModule = nullptr;
            compartment = compartmentIn;

            nextMemoryBase = 0;
            nextStackPointer = 0;
            nextTableBase = 0;
        }

        void setUser(const std::string &userIn) {
            user = userIn;
        }

        void setMainModule(Runtime::ModuleInstance *mainModuleIn) {
            mainModule = mainModuleIn;
        }

        /**
         * We can build the function GOT ahead of time just from the IR module.
         */
        void buildGOT(IR::Module &module) {
            // Extract disassembly names from the module
            IR::DisassemblyNames disassemblyNames;
            getDisassemblyNames(module, disassemblyNames);

            // Iterate through element segments to build map of table indices
            for (auto &es : module.elemSegments) {
                I32 offset = es.baseOffset.i32;

                for (Uptr i = 0; i < es.elems.size(); i++) {
                    const IR::Elem &elem = es.elems[i];

                    // Ignore null entries
                    if (!es.isActive && elem.type == IR::Elem::Type::ref_null) {
                        continue;
                    }

                    std::string &elemName = disassemblyNames.functions[elem.index].name;
                    int tableIdx = offset + i;
                    globalOffsetTableMap.insert(std::pair<std::string, int>(elemName, tableIdx));
                }
            }

            // Create a temporary map of global indices to initialisers
            // Note, first one will always be stack pointer which is mutable so we skip
            std::unordered_map<int, int> globalInitMap;
            for (Uptr i = 1; i < module.globals.defs.size(); i++) {
                IR::GlobalDef &global = module.globals.defs[i];

                if (global.type.isMutable) {
                    throw std::runtime_error("Haven't done this for mutables yet");
                }

                if (global.initializer.type != IR::InitializerExpression::Type::i32_const) {
                    throw std::runtime_error("Haven't done this for non-i32_const yet");
                }

                I32 value = global.initializer.i32;
                globalInitMap.insert(std::pair<int, int>(i, value));
            }

            // Iterate through exports and pick out the globals
            for (auto &ex : module.exports) {
                if (ex.kind != IR::ExternKind::global) {
                    continue;
                }

                // Get the initialiser value
                std::string globalName = disassemblyNames.globals[ex.index];
                int value = globalInitMap[ex.index];

                globalOffsetMemoryMap.insert(std::pair<std::string, int>(ex.name, value));
            }
        }

        void setUp(Runtime::Compartment *compartmentIn, IR::Module &module) {
            // Force memory sizes
            // module.memories.defs[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;
            module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;

            // Note, we don't want to mess with the min table size here, just give it room to expand if need be
            module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;

            // Build the global offset table
            this->buildGOT(module);

            envModule = Intrinsics::instantiateModule(compartmentIn, getIntrinsicModule_env(), "env");
        }

        void cleanUp() {
            envModule = nullptr;
        }

        bool resolve(const std::string &moduleName,
                     const std::string &name,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            bool isMainModule = mainModule == nullptr;

            if (isMainModule) {
                // Main module linking comes from env module
                resolved = getInstanceExport(envModule, name);
            } else {
                if (moduleName == "GOT.mem") {
                    // Here we're looking for a global memory offset
                    if (globalOffsetMemoryMap.count(name) == 0) {
                        logger->error("Failed to look up memory offset: {}.{}", moduleName, name);
                        return false;
                    }

                    int memOffset = globalOffsetMemoryMap[name];
                    logger->debug("Resolved {}.{} to {}", moduleName, name, memOffset);

                    Runtime::Global *gotMemoryOffset = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(gotMemoryOffset, memOffset);
                    resolved = asObject(gotMemoryOffset);

                } else if (moduleName == "GOT.func") {
                    // Here we're looking up a function table offset
                    if (globalOffsetTableMap.count(name) == 0) {
                        logger->error("Failed to look up function table index: {}.{}", moduleName, name);
                        return false;
                    }

                    int tableIdx = globalOffsetTableMap[name];
                    logger->debug("Resolved {}.{} to {}", moduleName, name, tableIdx);

                    Runtime::Global *gotFunctionOffset = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(gotFunctionOffset, tableIdx);
                    resolved = asObject(gotFunctionOffset);
                } else if (name == "__memory_base") {
                    // This is the point at which globals will be copied in
                    Runtime::Global *newMemoryBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newMemoryBase, nextMemoryBase);
                    resolved = asObject(newMemoryBase);
                } else if (name == "__table_base") {
                    // This is the offset in the imported table this module should use
                    Runtime::Global *newTableBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newTableBase, nextTableBase);
                    resolved = asObject(newTableBase);
                } else if (name == "__stack_pointer") {
                    // This is where the module should put its stack
                    Runtime::Global *newStackPointer = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newStackPointer, nextStackPointer);
                    resolved = asObject(newStackPointer);
                } else if (name == "__indirect_function_table") {
                    // This is the table shared with the main module. We will have
                    // made sure it is the right size with the right offset.
                    Runtime::Table *table = Runtime::getDefaultTable(mainModule);
                    resolved = asObject(table);
                } else {
                    // Look in normal env
                    resolved = getInstanceExport(envModule, name);

                    // If not resolved here, check on the main module
                    if (!resolved) {
                        resolved = getInstanceExport(mainModule, name);
                    }
                }
            }

            // Check whether the function has been resolved to the correct type
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

        int nextMemoryBase;
        int nextTableBase;
        int nextStackPointer;
    private:
        // Main module (not mastered here)
        Runtime::ModuleInstance *mainModule;

        Runtime::Compartment *compartment;

        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        std::string user;

        std::unordered_map<std::string, int> globalOffsetTableMap;
        std::unordered_map<std::string, int> globalOffsetMemoryMap;
    };

}