#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>

#include <proto/faasm.pb.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
    DECLARE_INTRINSIC_MODULE(env);

    const std::string ENTRYPOINT_FUNC = "_start";

    // Note that the max memory per module is 8GiB, i.e. > 100k pages
    // Page size in wasm is 64kiB so 1000 pages ~ 60MiB of memory
    const int MIN_MEMORY_PAGES = 1000;

    const int MAX_INPUT_BYTES = IR::numBytesPerPage;
    const int MAX_OUTPUT_BYTES = IR::numBytesPerPage;

    struct RootResolver : Runtime::Resolver {
        HashMap<std::string, Runtime::ModuleInstance*> moduleNameToInstanceMap;

        bool resolve(const std::string& moduleName,
                     const std::string& exportName,
                     IR::ExternType type,
                     Runtime::Object*& outObject) override
        {
            auto namedInstance = moduleNameToInstanceMap.get(moduleName);

            if(namedInstance) {
                outObject = getInstanceExport(*namedInstance, exportName);

                if(outObject && isA(outObject, type)) {
                    return true;
                };
            }

            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Missing import {}.{} {}", moduleName, exportName, asString(type).c_str());

            return false;
        }
    };

    class CallChain {
    public:
        explicit CallChain(const message::FunctionCall &call);

        void addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData);

        std::string execute();

        std::vector<message::FunctionCall> calls;
    private:
        const message::FunctionCall &originalCall;
    };

    class WasmModule {
    public:
        WasmModule();

        ~WasmModule();

        static std::vector<uint8_t> compile(message::FunctionCall &call);

        static void compileToObjectFile(message::FunctionCall &call);

        void initialise();

        int execute(message::FunctionCall &call, CallChain &callChain);

        Runtime::Memory *defaultMemory;
    private:
        IR::Module module;

        Runtime::Context * context = nullptr;
        Runtime::Compartment * compartment = nullptr;
        RootResolver *resolver = nullptr;

        Runtime::ModuleInstance *load(message::FunctionCall &call);

        void parseWasm(message::FunctionCall &call);
    };

    WasmModule *getExecutingModule();

    message::FunctionCall *getExecutingCall();

    CallChain *getExecutingCallChain();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode) : exitCode(exitCode) {

        }

        int exitCode;
    };
}