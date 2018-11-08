#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        long lossSize = memory->getStateSize(LOSSES_KEY);

        auto buffer = new uint8_t[lossSize];
        memory->readState(LOSSES_KEY, buffer, lossSize);

        auto losses = reinterpret_cast<double *>(buffer);
        long nLosses = lossSize / sizeof(double);

        std::string lossString;
        for (long l = 0; l < nLosses; l++) {
            lossString += std::to_string(losses[l]);

            if (l < nLosses - 1) {
                lossString += ", ";
            }
        }

        memory->setOutput((uint8_t *) lossString.data(), lossString.size());

        return 0;
    }
}