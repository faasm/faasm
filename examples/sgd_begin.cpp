#include "faasm.h"
#include "matrix.h"
#include "random.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        printf("Running SGD\n");

        const char *weightsKey = "weights";
        const char *inputsKey = "inputs";
        const char *outputsKey = "outputs";

        const char *realWeightsKey = "realWeights";

        int nTrain = 1000; // Number of training examples
        int nWeights = 10; // Number of weight factors

        // Create fake training data as dot product of the matrix of
        // training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, nWeights);
        Eigen::MatrixXd inputs = randomSparseMatrix(nWeights, nTrain);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, nWeights);

        // Write all data to memory
        writeMatrixState(memory, outputsKey, outputs);
        writeMatrixState(memory, inputsKey, inputs);
        writeMatrixState(memory, weightsKey, weights);

        // Also write real weights for safe keeping
        writeMatrixState(memory, realWeightsKey, realWeights);

        // Chain new calls to perform the work
        int nWorkers = 1;
        int batchSize = 100;
        for (int w = 0; w < nWorkers; w++) {
            int startIdx = (w * batchSize);
            int endIdx = startIdx + batchSize -1;

            int inputData[3] = {startIdx, endIdx, nWeights};
            int nBytes = 3 * sizeof(int);
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);

            memory->chainFunction("sgd_update", inputBytes, nBytes);
        }

        return 0;
    }
}