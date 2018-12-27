#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/matrix.h"
#include "faasm/sgd.h"

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

        // Load params
        SgdParams sgdParams = readParamsFromState(memory, PARAMS_KEY, REUTERS_FULL_ASYNC);

        // Always load the inputs and outputs async (as they should be constant)
        Map<SparseMatrix<double>> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx, true);
        Map<MatrixXd> outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, sgdParams.nTrain, startIdx, endIdx, 1,
                true);

        // Perform updates
        MatrixXd prediction;
        if (sgdParams.lossType == HINGE) {
            prediction = hingeLossWeightUpdate(memory, sgdParams, epoch, inputs, outputs);

            // Persist error
            writeHingeError(memory, sgdParams, batchNumber, outputs, prediction);
        } else {
            prediction = leastSquaresWeightUpdate(memory, sgdParams, inputs, outputs);

            // Persist error for these examples
            writeSquaredError(memory, sgdParams, batchNumber, outputs, prediction);
        }

        // Flag that this worker has finished
        writeFinishedFlag(memory, sgdParams, batchNumber);

        // If this is the last, dispatch the barrier (will have finished by now or will do soon)
        if (batchNumber == sgdParams.nBatches - 1) {
            memory->chainFunction("sgd_barrier");
        }

        return 0;
    }
}