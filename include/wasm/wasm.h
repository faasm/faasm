#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <string>
#include <exception>
#include <tuple>
#include <thread>

#include <boost/filesystem.hpp>

#include <proto/faasm.pb.h>

#include "Runtime/Intrinsics.h"
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"


namespace wasm {
    const std::string ENTRYPOINT_FUNC = "run";

    const int MAX_NAME_LENGTH = 32;

    // Input memory
    // Need to make sure this starts high enough to avoid other regions of the default memory
    const int INPUT_START = 1024 * 1024;
    const int MAX_INPUT_BYTES = 1024;

    // Output memory
    const int OUTPUT_START = INPUT_START + MAX_INPUT_BYTES;
    const int MAX_OUTPUT_BYTES = 1024;

    // Chaining memory
    const int MAX_CHAINS = 50;
    const int CHAIN_NAMES_START = OUTPUT_START + MAX_OUTPUT_BYTES;
    const int MAX_CHAIN_NAME_BYTES = MAX_NAME_LENGTH * MAX_CHAINS;

    const int CHAIN_DATA_START = CHAIN_NAMES_START + MAX_CHAIN_NAME_BYTES;

    /** Defines everything to do with a Wasm module */
    class WasmModule {
    public:
        WasmModule();

        /** Executes the function and stores the result */
        int execute(message::FunctionCall &call);

        /** Cleans up */
        void clean();

        std::vector<message::FunctionCall> chainedCalls;

    private:
        IR::Module module;

        IR::ValueTuple functionResults;

        static void compile(message::FunctionCall &call);

        Runtime::ModuleInstance* load(message::FunctionCall &call);
        void parseWasm(message::FunctionCall &call);
        void setUpMemory(message::FunctionCall &call);
        Runtime::LinkResult link(Runtime::Compartment *compartment);

        std::vector<IR::Value> buildInvokeArgs();

        void addDataSegment(int offset);
        void addInputData(Runtime::ModuleInstance *moduleInstance, message::FunctionCall &call);

        void extractOutputData(Runtime::ModuleInstance* moduleInstance, message::FunctionCall &call);
        void extractChainingData(Runtime::ModuleInstance* moduleInstance, const message::FunctionCall &call);
    };
}