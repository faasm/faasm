#include "faasm.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        int batchSize = 10;

        const uint8_t *input = memory->getInput();
        const int *trainingBatch = reinterpret_cast<const int*>(input);

        

        return 0;
    }
}