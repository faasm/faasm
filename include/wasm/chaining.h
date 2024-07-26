#pragma once

#include <faabric/proto/faabric.pb.h>

#include <cstdint>
#include <string>
#include <vector>

namespace wasm {

int awaitChainedCall(unsigned int messageId);

faabric::Message awaitChainedCallOutput(unsigned int messageId);

int makeChainedCall(const std::string& functionName,
                    int wasmFuncPtr,
                    const char* pyFunc,
                    const std::vector<uint8_t>& inputData);
}
