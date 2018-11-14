#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        size_t nBytes = p.nEpochs * sizeof(double);
        auto buffer = new uint8_t[nBytes];
        memory->readState(LOSSES_KEY, buffer, nBytes);

        auto losses = reinterpret_cast<double *>(buffer);

        std::string lossString;
        for (long l = 0; l < p.nEpochs; l++) {
            lossString += std::to_string(losses[l]);

            if (l < p.nEpochs - 1) {
                lossString += ", ";
            }
        }

        memory->setOutput((uint8_t *) lossString.data(), lossString.size());

        return 0;
    }
}