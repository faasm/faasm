#pragma once

#include <infra/infra.h>
#include <util/util.h>

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

    const std::string ENTRYPOINT_FUNC = "run";

    const int MAX_NAME_LENGTH = 32;

    // Note that the max memory per module is 8GiB, i.e. > 100k pages
    // Page size in wasm is 64kiB so 1000 pages ~ 60MiB of memory
    const int MIN_MEMORY_PAGES = 1000;

    // Input memory. Put this at the end of the defined range to avoid stuff trampling on it
    const int INPUT_START = (MIN_MEMORY_PAGES - 2) * IR::numBytesPerPage;
    const int MAX_INPUT_BYTES = 1024;

    // Output memory
    const int OUTPUT_START = INPUT_START + MAX_INPUT_BYTES;
    const int MAX_OUTPUT_BYTES = 1024;

    // Chaining memory
    const int MAX_CHAINS = 50;
    const int CHAIN_NAMES_START = OUTPUT_START + MAX_OUTPUT_BYTES;
    const int MAX_CHAIN_NAME_BYTES = MAX_NAME_LENGTH * MAX_CHAINS;

    const int CHAIN_DATA_START = CHAIN_NAMES_START + MAX_CHAIN_NAME_BYTES;

    Runtime::MemoryInstance* getModuleMemory();

    class WasmModule {
    public:
        WasmModule();

        /** Executes the function and stores the result */
        int execute(message::FunctionCall &call);

        /** Cleans up */
        void clean();

        /** Compiles the function to an object file */
        static std::vector<uint8_t> compile(message::FunctionCall &call);

        /** List of chained function calls */
        std::vector<message::FunctionCall> chainedCalls;

    private:
        IR::Module module;

        IR::ValueTuple functionResults;

        Runtime::ModuleInstance *load(message::FunctionCall &call, Runtime::Compartment *compartment);

        void parseWasm(message::FunctionCall &call);

        void setUpMemoryDefinitions(message::FunctionCall &call);

        Runtime::LinkResult link(Runtime::Compartment *compartment);

        std::vector<IR::Value> buildInvokeArgs();

        void addDataSegment(int offset);

        void addInputData(message::FunctionCall &call);

        void extractOutputData(message::FunctionCall &call);

        void extractChainingData(const message::FunctionCall &call);
    };
}