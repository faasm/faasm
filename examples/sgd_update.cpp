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

        // Calculate total squared error
        double squaredError = 0;
        for(int e = 0; e < error.cols(); e++) {
            squaredError += e^2;
        }
        printf("SQUARED ERROR: %.2f\n", squaredError);

        MatrixXd gradient = error * inputs.transpose();
        gradient *= (-2.0/nFactors);
        for(int i = 0; i < nFactors; i++) {
            double grad = gradient(0, i);

            // Ignore very small
            if(abs(grad) <= 0.0001) {
                continue;
            }

            // Make the update
            double before = weights(0, i);
            weights(0, i) -= learningRate * grad;

            writeMatrixStateElement(memory, weightsKey, weights, 0, i);

            double after = weights(0, i);
            printf("UPDATE: %.2f -> %.2f (%.8f)\n", before, after, grad);
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