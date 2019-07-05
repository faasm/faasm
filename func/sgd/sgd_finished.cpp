#include "faasm/faasm.h"
#include "faasm/sgd.h"
#include "faasm/counter.h"

#include <string>

void _setOutput(const char *result) {
    auto resultBytes = reinterpret_cast<const uint8_t *>(result);
    faasmSetOutput(resultBytes, strlen(result));
}

FAASM_MAIN_FUNC() {
    bool fullAsync = faasm::getEnvFullAsync();
    faasm::SgdParams p = faasm::readParamsFromState(PARAMS_KEY, fullAsync);
    int epochCount = faasm::getCounter(EPOCH_COUNT_KEY, fullAsync);

    // Work out if we've finished all epochs
    if (epochCount >= p.nEpochs - 1) {
        const char *result = "true";
        _setOutput(result);
    } else {
        const char *result = "false";
        _setOutput(result);
    }

    return 0;
}