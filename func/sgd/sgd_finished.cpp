#include "faasm/faasm.h"
#include "faasm/sgd.h"
#include "faasm/counter.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);

        int epochCount = faasm::getCounter(memory, EPOCH_COUNT_KEY, REUTERS_FULL_ASYNC);

        // Work out if we've finished all epochs
        if (epochCount >= p.nEpochs - 1) {
            memory->setOutput((uint8_t *) "true", 4);
        } else {
            memory->setOutput((uint8_t *) "false", 5);
        }

        return 0;
    }
}