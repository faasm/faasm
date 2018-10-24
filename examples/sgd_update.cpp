#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int *>(input);

        int startIdx = inputParams[0];
        int endIdx = inputParams[1];
        int nFactors = inputParams[2];

        printf("Epoch for %d to %d with %d factors\n", startIdx, endIdx, nFactors);

        const double learningRate = 1.0;

        // Read in the state
        const char *inputsKey = "inputs";
        const char *outputsKey = "outputs";
        const char *weightsKey = "weights";
        const char *realWeightsKey = "realWeights";

        // Load the current weights
        MatrixXd weights = readMatrixFromState(memory, weightsKey, 1, nFactors);
        MatrixXd realWeights = readMatrixFromState(memory, realWeightsKey, 1, nFactors);

        // Get matching inputs and outputs
        MatrixXd inputs = readMatrixColumnsFromState(memory, inputsKey, startIdx, endIdx, nFactors);
        MatrixXd outputs = readMatrixColumnsFromState(memory, outputsKey, startIdx, endIdx, 1);

        // Work out what these weights would actually give us
        MatrixXd actual = weights * inputs;

        // Get the error
        MatrixXd error = actual - outputs;

        // Calculate MSE
        ArrayXd squaredError = error.transpose().array().pow(2.0);
        double mse = squaredError.sum();
        printf("MSE: %.4f\n", mse);

        // Work out the step size
        MatrixXd gradient = (2.0 * learningRate) * error * inputs.transpose();

        // Perform updates to weights
        for(int i = 0; i < nFactors; i++) {
            double stepSize = gradient(0, i);

            // Ignore (effectively) zero gradients
            if(abs(stepSize) <= 0.00001) {
                continue;
            }

            // Make the update
            weights(0, i) -= stepSize;
            writeMatrixStateElement(memory, weightsKey, weights, 0, i);
        }

        // Print out updated weights
        for(int w = 0; w < nFactors; w++) {
            double guess = weights(0, w);
            double real = realWeights(0, w);

            printf("%.2f (%.2f)\n", guess, real);
        }

        return 0;
    }
}