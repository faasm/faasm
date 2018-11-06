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

    // TODO: avoid the dodgy funciton naming?
    // The CPP compiler seems to mangle the run function export name
    const std::string ENTRYPOINT_FUNC = "_start";

    // Note that the max memory per module is 8GiB, i.e. > 100k pages
    // Page size in wasm is 64kiB so 1000 pages ~ 60MiB of memory
    const int MIN_MEMORY_PAGES = 1000;

    const int MAX_INPUT_BYTES = IR::numBytesPerPage;
    const int MAX_OUTPUT_BYTES = IR::numBytesPerPage;

    class CallChain {
    public:
        CallChain(const message::FunctionCall &call);

        void addCall(const std::string &user, const std::string &functionName, const std::vector<uint8_t> &inputData);
        std::string execute();

        std::vector<message::FunctionCall> calls;
    private:
        const message::FunctionCall &originalCall;
    };

    class WasmModule {
    public:
        WasmModule(message::FunctionCall &call);

        static std::vector<uint8_t> compile(message::FunctionCall &call);
        static void compileToObjectFile(message::FunctionCall &call);

        int execute();

        Runtime::Memory* defaultMemory;
        message::FunctionCall functionCall;
        CallChain callChain;
    private:
        IR::Module module;

        IR::ValueTuple functionResults;

        Runtime::ModuleInstance *load(Runtime::Compartment *compartment);

        void parseWasm();

        Runtime::LinkResult link(Runtime::Compartment *compartment);

        void validateInputData();
    };

    WasmModule * getExecutingModule();

    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode): exitCode(exitCode) {

        }

        int exitCode;
    };
}