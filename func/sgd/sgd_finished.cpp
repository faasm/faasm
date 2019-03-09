#include "faasm/faasm.h"
#include "faasm/sgd.h"
#include "faasm/counter.h"

#include <string>

namespace faasm {
    void _setOutput(FaasmMemory *memory, const char *result) {
        auto resultBytes = reinterpret_cast<const uint8_t *>(result);
        memory->setOutput(resultBytes, strlen(result));
    }

    int exec(FaasmMemory *memory) {
        bool fullAsync = getEnvFullAsync();
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, fullAsync);
        int epochCount = faasm::getCounter(memory, EPOCH_COUNT_KEY, fullAsync);

        // Work out if we've finished all epochs
        if (epochCount >= p.nEpochs - 1) {
            const char *result = "true";
            _setOutput(memory, result);
        } else {
            const char *result = "false";
            _setOutput(memory, result);
        }

        return 0;
    }
}