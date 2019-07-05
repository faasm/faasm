#include "faasm/faasm.h"
#include "faasm/print.h"
#include "faasm/sgd.h"

#include <string>

FAASM_MAIN_FUNC() {
    bool fullAsync = faasm::getEnvFullAsync();
    faasm::SgdParams p = faasm::readParamsFromState(PARAMS_KEY, fullAsync);

    // Get loss values from state
    size_t lossBytes = p.nEpochs * sizeof(double);
    auto lossBuffer = new uint8_t[lossBytes];
    faasmReadState(LOSSES_KEY, lossBuffer, lossBytes, p.fullAsync);
    auto losses = reinterpret_cast<double *>(lossBuffer);

    // Get loss timestamps from state
    size_t tsBytes = p.nEpochs * sizeof(double);
    auto tsBuffer = new uint8_t[tsBytes];
    faasmReadState(LOSS_TIMESTAMPS_KEY, tsBuffer, tsBytes, p.fullAsync);
    auto ts = reinterpret_cast<double *>(tsBuffer);

    // Build string representation
    char lossString[p.nEpochs * 20];
    size_t offset = 0;
    for (long l = 0; l < p.nEpochs; l++) {
        // Want relative times
        double relativeTs = ts[l] - ts[0];

        // Overwrite the last null terminator
        char *tsString = faasm::floatToStr(relativeTs);
        char *thisLossString = faasm::floatToStr(losses[l]);
        sprintf(lossString + offset, "%s - %s, ", tsString, thisLossString);
        offset = strlen(lossString);
    }

    // Set as output
    size_t outputLen = strlen(lossString);
    faasmSetOutput((uint8_t *) lossString, outputLen);

    return 0;
}
