#include "faasm.h"
#include "matrix.h"
#include "counter.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        int nWeights = 10;
        int nTrain = 1000;
        int nWorkers = 10;
        int batchSize = nTrain / nWorkers;

        // Set all worker counts to zero
        for (int i = 0; i < nWorkers; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);
            initCounter(memory, workerKey);
        }

        // Chain new calls to perform the work
        for (int w = 0; w < nWorkers; w++) {
            int startIdx = (w * batchSize);
            int endIdx = startIdx + batchSize - 1;

            // Prepare input data for the worker
            int inputData[4] = {w, startIdx, endIdx, nWeights};
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);
            int nBytes = 4 * sizeof(int);

            // Call the chained function
            memory->chainFunction("sgd_step", inputBytes, nBytes);
        }

        // Dispatch the barrier function
        uint8_t barrierInput[1] = {0};
        memory->chainFunction("sgd_barrier", barrierInput, 1);

        return 0;
    }
}