#include "faasm/core.h"
#include "faasm/pool.h"

namespace faasm {
    unsigned int FunctionPool::chain(int funcIdx) {
        std::vector<uint8_t> empty;
        unsigned int callId = faasmChainThisInput(funcIdx, empty.data(), 0);

        callIds.emplace_back(callId);

        return callId;
    }

    void FunctionPool::awaitAll() {
        for(auto callId : callIds) {
            faasmAwaitCall(callId);
        }
    }
}
