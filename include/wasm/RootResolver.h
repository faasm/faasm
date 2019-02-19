#pragma once

#include <util/logging.h>

#include <string>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
    // Standard toolchain version
    extern Intrinsics::Module &getIntrinsicModule_env();

    // Emscripten versions
    extern Intrinsics::Module &getIntrinsicModule_emEnv();

    extern Intrinsics::Module &getIntrinsicModule_emAsm2wasm();

    extern Intrinsics::Module &getIntrinsicModule_emGlobal();

    struct RootResolver : Runtime::Resolver {
        explicit RootResolver(Runtime::Compartment *compartment) {
            envModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_env(), "env");

            emEnvModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emEnv(), "emEnv");
            emAsm2wasmModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emAsm2wasm(), "emAsm2wasm");
            emGlobalModule = Intrinsics::instantiateModule(compartment, getIntrinsicModule_emGlobal(), "emGlobal");

        }

        void setUser(const std::string &userIn) {
            user = userIn;
        }

        void setIsEmscripten(bool isEmscriptenIn) {
            isEmscripten = isEmscriptenIn;
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

    private:
        Runtime::GCPointer<Runtime::ModuleInstance> envModule;

        // Emscripten modules
        Runtime::GCPointer<Runtime::ModuleInstance> emEnvModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emAsm2wasmModule;
        Runtime::GCPointer<Runtime::ModuleInstance> emGlobalModule;

        std::string user;

        bool isEmscripten = false;
    };
}