#include "faasm/faasm.h"
#include "sgd_constants.h"
#include "faasm/counter.h"
#include "faasm/matrix.h"

namespace faasm {
    MatrixXd doWeightUpdate(FaasmMemory *memory, MatrixXd &weights, const MatrixXd &inputs, const MatrixXd &outputs) {
        // Work out what the current weights would actually give us
        MatrixXd actual = weights * inputs;

        // Work out the error
        MatrixXd error = actual - outputs;

        // Work out the step size
        MatrixXd steps = (2.0 * LEARNING_RATE) * error * inputs.transpose();

        // Perform updates to weights
        for (int i = 0; i < N_WEIGHTS; i++) {
            double stepSize = steps(0, i);

            // Ignore (effectively) zero gradients
            if (abs(stepSize) <= 0.00001) {
                continue;
            }

            // Make the update
            weights(0, i) -= stepSize;
            writeMatrixStateElement(memory, WEIGHTS_KEY, weights, 0, i);
        }

        return actual;
    }

    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int *>(input);

        int workerIdx = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];

        // Load the data
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, N_WEIGHTS);
        MatrixXd inputs = readMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx, N_WEIGHTS);
        MatrixXd outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, startIdx, endIdx, 1);

        // Perform updates
        MatrixXd actual = doWeightUpdate(memory, weights, inputs, outputs);

        // Calculate error
        char batchKey[10];
        sprintf(batchKey, "batch-%i", workerIdx);
        double squaredError = calculateSquaredError(actual, outputs);
        auto squaredErrorBytes = reinterpret_cast<uint8_t *>(&squaredError);
        memory->writeState(batchKey, squaredErrorBytes, sizeof(double));

        return 0;
    }
}