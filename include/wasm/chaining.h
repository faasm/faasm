#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wasm {

int awaitChainedCall(unsigned int messageId);

int awaitChainedCallOutput(unsigned int messageId, char* buffer, int bufferLen);

int makeChainedCall(const std::string& functionName,
                    int wasmFuncPtr,
                    const char* pyFunc,
                    const std::vector<uint8_t>& inputData);
}
