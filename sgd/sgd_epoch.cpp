#include "faasm.h"
#include "matrix.h"
#include "counter.h"
#include "sgd_constants.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        int batchSize = N_TRAIN / N_BATCHES;

        // Set all worker counts to zero
        for (int i = 0; i < N_BATCHES; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);
            initCounter(memory, workerKey);
        }

        // Chain new calls to perform the work
        for (int w = 0; w < N_BATCHES; w++) {
            int startIdx = (w * batchSize);
            int endIdx = startIdx + batchSize - 1;

            // Prepare input data for the worker
            int inputData[4] = {w, startIdx, endIdx, N_WEIGHTS};
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