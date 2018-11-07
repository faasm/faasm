#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Load inputs and params
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);
        MatrixXd inputs = readMatrixFromState(memory, INPUTS_KEY, p.nWeights, p.nTrain);
        MatrixXd outputs = readMatrixFromState(memory, OUTPUTS_KEY, p.nWeights, p.nTrain);

        // Shuffle inputs and outputs
        faasm::shufflePairedMatrixColumns(inputs, outputs);
        writeMatrixState(memory, INPUTS_KEY, inputs);
        writeMatrixState(memory, OUTPUTS_KEY, outputs);

        // Set all batch errors to zero
        for (int i = 0; i < p.nBatches; i++) {
            char batchKey[10];
            sprintf(batchKey, "batch-%i", i);
            uint8_t batchError[1] = {0};
            memory->writeState(batchKey, batchError, 1);
        }

        // Chain new calls to perform the work
        int batchSize = p.nTrain / p.nBatches;
        for (int w = 0; w < p.nBatches; w++) {
            int startIdx = (w * batchSize);
            int endIdx = startIdx + batchSize - 1;

            // Prepare input data for the worker
            int inputData[3] = {w, startIdx, endIdx};
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);
            int nBytes = 3 * sizeof(int);

            // Call the chained function
            memory->chainFunction("sgd_step", inputBytes, nBytes);
        }

        // Dispatch the barrier function
        memory->chainFunction("sgd_barrier");

        return 0;
    }
}