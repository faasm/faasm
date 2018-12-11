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
        SgdParams sgdParams = readParamsFromState(memory, PARAMS_KEY);

        // Load the data
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, sgdParams.nWeights);
        SparseMatrix<double> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx);
        MatrixXd outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, startIdx, endIdx, 1);

        // Perform updates
        MatrixXd actual;
        if (sgdParams.lossType == HINGE) {
            actual = hingeLossWeightUpdate(memory, sgdParams, epoch, weights, inputs, outputs);

            // Persist error
            writeHingeError(memory, batchNumber, outputs, actual);
        } else {
            actual = leastSquaresWeightUpdate(memory, sgdParams, weights, inputs, outputs);

            // Persist error for these examples
            writeSquaredError(memory, batchNumber, outputs, actual);
        }

        // Flag that this worker has finished
        writeFinishedFlag(memory, batchNumber);

        // If this is the last, dispatch the barrier (will have finished by now or will do soon)
        if(batchNumber == sgdParams.nBatches - 1) {
            memory->chainFunction("sgd_barrier");
        }

        return 0;
    }
}