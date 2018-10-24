#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int *>(input);

        int startIdx = inputParams[0];
        int endIdx = inputParams[1];
        int nFactors = inputParams[2];
        int batchSize = endIdx - startIdx + 1;

        printf("Epoch for %d to %d with %d factors (batch %d) \n", startIdx, endIdx, nFactors, batchSize);

        const double learningRate = 1.0;

        // Read in the state
        const char *inputsKey = "inputs";
        const char *outputsKey = "outputs";
        const char *weightsKey = "weights";
        const char *realWeightsKey = "realWeights";

        // Load the current weights
        printf("Reading weights\n");
        MatrixXd weights = readMatrixFromState(memory, weightsKey, 1, nFactors);
        MatrixXd realWeights = readMatrixFromState(memory, realWeightsKey, 1, nFactors);

        // Get matching inputs and outputs
        printf("Reading inputs/ outputs\n");
        MatrixXd inputs = readMatrixColumnsFromState(memory, inputsKey, startIdx, endIdx, nFactors);
        MatrixXd outputs = readMatrixColumnsFromState(memory, outputsKey, startIdx, endIdx, 1);

        // Work out what these weights would actually give us
        printf("Calculating actuals\n");
        MatrixXd actual = weights * inputs;

        // Get the error
        printf("Calculating error\n");
        MatrixXd error = actual - outputs;

        // Calculate MSE
        printf("Calculating MSE\n");
        double mse = 0;
        for(int r = 0; r < error.rows(); r++) {
            for(int c = 0; c < error.cols(); c++) {
                double e = error.coeff(r, c);
                mse += pow(e, 2);
            }
        }
        mse /= batchSize;
        printf("MSE: %.4f\n", mse);

        // Work out the step size
        printf("Calculating gradient\n");
        MatrixXd gradient = (2.0 * learningRate) * error * inputs.transpose();

        // Perform updates to weights
        for(int i = 0; i < nFactors; i++) {
            printf("Performing step %d\n", i);
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