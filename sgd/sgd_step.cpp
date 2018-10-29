#include "faasm.h"
#include "matrix.h"
#include "counter.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int *>(input);

        int workerIdx = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];
        int nWeights = inputParams[3];

        const double learningRate = 0.1;

        // Read in the state
        const char *inputsKey = "inputs";
        const char *outputsKey = "outputs";
        const char *weightsKey = "weights";

        // Load the current weights
        MatrixXd weights = readMatrixFromState(memory, weightsKey, 1, nWeights);

        // Get matching inputs and outputs
        MatrixXd inputs = readMatrixColumnsFromState(memory, inputsKey, startIdx, endIdx, nWeights);
        MatrixXd outputs = readMatrixColumnsFromState(memory, outputsKey, startIdx, endIdx, 1);

        // Work out what these weights would actually give us
        MatrixXd actual = weights * inputs;

        // Work out the step size
        MatrixXd error = actual - outputs;
        MatrixXd gradient = (2.0 * learningRate) * error * inputs.transpose();

        // Perform updates to weights
        for(int i = 0; i < nWeights; i++) {
            double stepSize = gradient(0, i);

            // Ignore (effectively) zero gradients
            if(abs(stepSize) <= 0.00001) {
                continue;
            }

            // Make the update
            weights(0, i) -= stepSize;
            writeMatrixStateElement(memory, weightsKey, weights, 0, i);
        }

        // Record that this worker has finished
        char workerKey[10];
        sprintf(workerKey, "worker-%i", workerIdx);
        incrementCounter(memory, workerKey);

        return 0;
    }
}