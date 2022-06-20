#include <wamr/native.h>
#include <wasm/chaining.h>

#include <vector>
#include <wasm_export.h>

namespace wasm {
static int32_t __faasm_chain_name_wrapper(wasm_exec_env_t execEnv,
                                          const char* name,
                                          const uint8_t* input,
                                          uint32_t inputSize)
{
    std::vector<uint8_t> _input(input, input + inputSize);
    return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
}

static NativeSymbol ns[] = { REG_NATIVE_FUNC(__faasm_chain_name, "($$i)i") };

uint32_t getFaasmChainingApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
