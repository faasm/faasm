#pragma once

#include <infra/infra.h>

#include <string>
#include <exception>
#include <tuple>

#include <proto/faasm.pb.h>
#include <Runtime/Runtime.h>

using namespace IR;
using namespace Runtime;

namespace worker {
    const std::string ENTRYPOINT_FUNC = "run";

    const int MAX_NAME_LENGTH = 32;

    // Input memory
    // Need to make sure this starts high enough to avoid
    // other bits of the default memory
    const int INPUT_START = 1024 * 20;
    const int MAX_INPUT_BYTES = 1024;

    // Output memory
    const int OUTPUT_START = INPUT_START + MAX_INPUT_BYTES;
    const int MAX_OUTPUT_BYTES = 1024;

    // Chaining memory
    const int MAX_CHAINS = 50;
    const int CHAIN_NAMES_START = OUTPUT_START + MAX_OUTPUT_BYTES;
    const int MAX_CHAIN_NAME_BYTES = MAX_NAME_LENGTH * MAX_CHAINS;

    const int CHAIN_DATA_START = CHAIN_NAMES_START + MAX_CHAIN_NAME_BYTES;
    const int MAX_CHAIN_DATA_BYTES = MAX_INPUT_BYTES * MAX_CHAINS;

    /** Wrapper for wasm code */
    class WasmModule {
    public:
        WasmModule();

        /** Executes the function and stores the result */
        int execute(message::FunctionCall &call);

        /** Cleans up */
        void clean();

        size_t getChainCount();
        std::string getChainName(const size_t &idx);
        std::vector<U8> getChainData(const size_t &idx);

    private:
        ModuleInstance *moduleInstance;
        ValueTuple functionResults;

        std::vector<std::string> chainNames;
        std::vector<std::vector<U8>> chainData;
    };

    /** Worker wrapper */
    class Worker {
    public:
        Worker();

        void start();
    private:
        infra::Redis redis;
    };

    /** Exceptions */
    class WasmException : public std::exception {
    };

    class InvalidResultException : public std::exception {
    };
}