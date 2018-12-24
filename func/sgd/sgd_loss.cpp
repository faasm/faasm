#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);

        // Get loss values from state
        size_t lossBytes = p.nEpochs * sizeof(double);
        auto lossBuffer = new uint8_t[lossBytes];
        memory->readState(LOSSES_KEY, lossBuffer, lossBytes, p.fullAsync);
        auto losses = reinterpret_cast<double *>(lossBuffer);

        // Get loss timestamps from state
        size_t tsBytes = p.nEpochs * sizeof(long);
        auto tsBuffer = new uint8_t[tsBytes];
        memory->readState(LOSS_TIMESTAMPS_KEY, tsBuffer, tsBytes, p.fullAsync);
        auto ts = reinterpret_cast<long *>(tsBuffer);

        long baseTs = ts[0];
        std::string lossString;
        for (long l = 0; l < p.nEpochs; l++) {
            long relativeTs = ts[l] - baseTs;
            lossString += std::to_string(relativeTs) + " - " + std::to_string(losses[l]);

            if (l < p.nEpochs - 1) {
                lossString += ", ";
            }
        }

        memory->setOutput((uint8_t *) lossString.data(), lossString.size());

        return 0;
    }
}