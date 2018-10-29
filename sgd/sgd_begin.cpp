#include "faasm.h"
#include "matrix.h"
#include "counter.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
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

        // Begin first epoch
        initCounter(memory, "epochCount");
        uint8_t epochInput[1] = {0};
        memory->chainFunction("sgd_epoch", epochInput, 1);

        return 0;
    }
}