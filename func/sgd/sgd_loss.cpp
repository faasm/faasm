#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Get loss values from state
        size_t lossBytes = p.nEpochs * sizeof(double);
        auto lossBuffer = new uint8_t[lossBytes];
        memory->readState(LOSSES_KEY, lossBuffer, lossBytes);
        auto losses = reinterpret_cast<double *>(lossBuffer);

        // Get loss timestamps from state
        size_t tsBytes = p.nEpochs * sizeof(long);
        auto tsBuffer = new uint8_t[tsBytes];
        memory->readState(LOSS_TIMESTAMPS_KEY, tsBuffer, tsBytes);
        auto ts = reinterpret_cast<long *>(tsBuffer);

        std::string lossString;
        for (long l = 0; l < p.nEpochs; l++) {
            lossString += std::to_string(ts[l]) + " - " + std::to_string(losses[l]);

            if (l < p.nEpochs - 1) {
                lossString += ", ";
            }
        }

        memory->setOutput((uint8_t *) lossString.data(), lossString.size());

        return 0;
    }
}