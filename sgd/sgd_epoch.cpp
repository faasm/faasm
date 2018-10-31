#include "faasm.h"
#include "sgd_constants.h"
#include "matrix.h"
#include "counter.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Load inputs
        MatrixXd inputs = faasm::readMatrixFromState(memory, INPUTS_KEY, N_WEIGHTS, N_TRAIN);

        // Shuffle and write
        faasm::shuffleMatrixColumns(inputs);
        writeMatrixState(memory, INPUTS_KEY, inputs);

        // Set all batch errors to zero
        for (int i = 0; i < N_BATCHES; i++) {
            char batchKey[10];
            sprintf(batchKey, "batch-%i", i);
            uint8_t batchError[1] = {0};
            memory->writeState(batchKey, batchError, 1);
        }

        // Chain new calls to perform the work
        int batchSize = N_TRAIN / N_BATCHES;
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
        memory->chainFunction("sgd_barrier");

        return 0;
    }
}