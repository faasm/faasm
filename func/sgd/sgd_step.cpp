#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/matrix.h"
#include "faasm/sgd.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        size_t nBytes = sizeof(int) * 4;
        auto inputBuffer = new uint8_t[nBytes];
        memory->getInput(inputBuffer, nBytes);

        const int *inputParams = reinterpret_cast<const int *>(inputBuffer);

        int batchNumber = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];
        int epoch = inputParams[3];

        printf("Starting batch %i (%i - %i). Epoch %i\n", batchNumber, startIdx, endIdx, epoch);

        // Load params
        printf("------- A\n");
        SgdParams sgdParams = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);

        // Always load the inputs and outputs async (as they should be constant)
        printf("------- B\n");
        Map<SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx, true);

        printf("------- C\n");
        Map<MatrixXd> outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, sgdParams.nTrain, startIdx, endIdx, 1,
                true);

        // Perform updates
        double *predictions;
        if (sgdParams.lossType == HINGE) {
            predictions = hingeLossWeightUpdate(memory, sgdParams, epoch, inputs, outputs);

            // Persist error
            printf("------- J\n");
            writeHingeError(memory, sgdParams, batchNumber, predictions, outputs);

        } else {
            predictions = leastSquaresWeightUpdate(memory, sgdParams, inputs, outputs);

            // Persist error for these examples
            writeSquaredError(memory, sgdParams, batchNumber, predictions, outputs);
        }

        delete[] predictions;

        // Flag that this worker has finished
        writeFinishedFlag(memory, sgdParams, batchNumber);

        // If this is the last, dispatch the barrier (will have finished by now or will do soon)
        if (batchNumber == sgdParams.nBatches - 1) {
            memory->chainFunction("sgd_barrier");
        }

        return 0;
    }
}