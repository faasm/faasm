#include <stdio.h>
#include <sgx/SGXWAMRWasmModule.h>

namespace wasm {
    extern int makeChainedCall(const std::string &functionName, int idx, const char *pyFuncName,
                               const std::vector<uint8_t> &inputData);

    extern int awaitChainedCall(unsigned int messageId);
}

extern "C" {

void ocall_sgx_wamr_example_native_symbol(const char *message) {
    printf("sgx_wamr_example_native_symbols: %s \n", message);
}

unsigned int ocall_faasm_chain_function_input(const char *name, const uint8_t *input, long input_size) {
    std::vector<uint8_t> _input(input, input + input_size);
    return wasm::makeChainedCall(std::string(name), 0, NULL, _input);
}

unsigned int ocall_faasm_chain_this_input(const int idx, uint8_t *input, long input_size) {
    faabric::Message *bounded_message = wasm::getExecutingCall();
    const std::vector<uint8_t> _input(input, input + input_size);
    return wasm::makeChainedCall(bounded_message->function(), idx, NULL, _input);
}

unsigned int ocall_faasm_await_call(unsigned int call_id) {
    return wasm::awaitChainedCall(call_id);
}

};