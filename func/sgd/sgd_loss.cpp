#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        bool fullAsync = getEnvFullAsync();
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, fullAsync);

        // Get loss values from state
        size_t lossBytes = p.nEpochs * sizeof(double);
        auto lossBuffer = new uint8_t[lossBytes];
        memory->readState(LOSSES_KEY, lossBuffer, lossBytes, p.fullAsync);
        auto losses = reinterpret_cast<double *>(lossBuffer);

        // Get loss timestamps from state
        size_t tsBytes = p.nEpochs * sizeof(double);
        auto tsBuffer = new uint8_t[tsBytes];
        memory->readState(LOSS_TIMESTAMPS_KEY, tsBuffer, tsBytes, p.fullAsync);
        auto ts = reinterpret_cast<double *>(tsBuffer);

        // Build string representation
        char lossString[p.nEpochs * 20];
        size_t offset = 0;
        for (long l = 0; l < p.nEpochs; l++) {
            // Want relative times
            double relativeTs = ts[l] - ts[0];

            // Overwrite the last null terminator
            sprintf(lossString + offset, "%.2f - %.4f, ", relativeTs, losses[l]);
            offset = strlen(lossString);
        }

        // Set as output
        size_t outputLen = strlen(lossString);
        memory->setOutput((uint8_t *) lossString, outputLen);

        return 0;
    }
}