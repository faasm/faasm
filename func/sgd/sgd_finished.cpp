#include "faasm/faasm.h"
#include "faasm/sgd.h"

#include <string>

namespace faasm {
    int exec(FaasmMemory *memory) {
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);
        bool isFinished = readEpochFinished(memory, p);

        if (isFinished) {
            memory->setOutput((uint8_t *) "true", 4);
        } else {
            memory->setOutput((uint8_t *) "false", 5);
        }

        return 0;
    }
}