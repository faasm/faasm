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
    // Note that page size in wasm is 64kiB
    // Note also that this initial memory must be big enough to include all data, stack and dynamic
    // memory that the module will need.
    const int ONE_MB_PAGES = 16;
    const int ONE_GB_PAGES = 1024 * ONE_MB_PAGES;

    const int INITIAL_MEMORY_PAGES = 128 * ONE_MB_PAGES;
    const int MAX_MEMORY_PAGES = ONE_GB_PAGES;

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

        void setUp(Runtime::Compartment *compartmentIn, IR::Module &module) {
            // Force memory sizes
            module.memories.defs[0].type.size.min = (U64) INITIAL_MEMORY_PAGES;
            module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;

            // Note, we don't want to mess with the min table size here, just give it room to expand if need be
            module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;

            envModule = Intrinsics::instantiateModule(compartmentIn, getIntrinsicModule_env(), "env");
        }

        void cleanUp() {
            envModule = nullptr;
        }

        bool resolve(const std::string &moduleName,
                     const std::string &exportName,
                     IR::ExternType type,
                     Runtime::Object *&resolved) override {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            
            bool isDynamicModule = mainModule != nullptr;
            
            if(isDynamicModule) {
                // The special cases below are the globals that are crucial to getting the
                // dynamic linking to work. 
                if(exportName == "__memory_base") {
                    // This is the point at which globals will be copied in
                    Runtime::Global *newMemoryBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newMemoryBase, nextMemoryBase);
                    resolved = asObject(newMemoryBase);
                }
                else if(exportName == "__table_base") {
                    // This is the offset in the imported table this module should use
                    Runtime::Global *newTableBase = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newTableBase, nextTableBase);
                    resolved = asObject(newTableBase);
                } else if(exportName == "__stack_pointer") {
                    // This is where the module should put its stack
                    Runtime::Global *newStackPointer = Runtime::createGlobal(compartment, asGlobalType(type));
                    Runtime::initializeGlobal(newStackPointer, nextStackPointer);
                    resolved = asObject(newStackPointer);
                }
                else if(exportName == "__indirect_function_table") {
                    // This is the table shared with the main module. We will have
                    // made sure it is the right size with the right offset.
                    Runtime::Table *table = Runtime::getDefaultTable(mainModule);
                    resolved = asObject(table);
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

        int nextMemoryBase;
        int nextTableBase;
        int nextStackPointer;
    private:
        // Main module (not mastered here)
        Runtime::ModuleInstance *mainModule;

        Runtime::Compartment *compartment;

        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        std::string user;
    };

}