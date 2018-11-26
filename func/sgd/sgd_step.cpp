#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/matrix.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        size_t nBytes = sizeof(int) * 3;
        auto inputBuffer = new uint8_t[nBytes];
        memory->getInput(inputBuffer, nBytes);

        const int *inputParams = reinterpret_cast<const int *>(inputBuffer);

        int workerIdx = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];

        // Load params
        SgdParams sgdParams = readParamsFromState(memory, PARAMS_KEY);

        // Load the data
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, sgdParams.nWeights);
        SparseMatrix<double> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx);
        MatrixXd outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, startIdx, endIdx, 1);

        // Perform updates
        MatrixXd actual = leastSquaresWeightUpdate(memory, sgdParams, weights, inputs, outputs);

        // Persist error for these examples
        writeSquaredError(memory, workerIdx, outputs, actual);

        // Flag that this worker has finished
        writeFinishedFlag(memory, workerIdx);

        return 0;
    }
}