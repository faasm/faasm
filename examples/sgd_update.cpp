#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int*>(input);

        int startIdx = inputParams[0];
        int endIdx = inputParams[1];
        int nFactors = inputParams[2];
        int batchSize = endIdx - startIdx;

        printf("Epoch for %d to %d with %d factors\n", startIdx, endIdx, nFactors);

        // Read in the state
        const char* inputsKey = "inputs";
        const char* outputsKey = "outputs";
        const char* weightsKey = "weights";

        // Load the current weights
        MatrixXd weights = readMatrixFromState(memory, weightsKey, 1, nFactors);

        // Get matching inputs and outputs
        MatrixXd inputs = readMatrixColumnsFromState(memory, inputsKey, startIdx, endIdx, nFactors);
        MatrixXd outputs = readMatrixColumnsFromState(memory, outputsKey, startIdx, endIdx, 1);

        // Iterate through all training examples in the batch
        for(int b =0; b < batchSize; b++) {
            VectorXd inputVec = inputs.col(b);
            VectorXd outputVec = outputs.col(b);

            VectorXd actual = weights * inputVec;
            VectorXd difference = outputVec - actual;
        }

        return 0;
    }
}