#include "faasm.h"
#include "matrix.h"
#include "counter.h"
#include "sgd_constants.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Create fake training data as dot product of the matrix of
        // training input data and the real weight vector
        Eigen::MatrixXd realWeights = randomDenseMatrix(1, N_WEIGHTS);
        Eigen::MatrixXd inputs = randomSparseMatrix(N_WEIGHTS, N_TRAIN);
        Eigen::MatrixXd outputs = realWeights * inputs;

        // Initialise a random set of weights that we'll train (i.e. these should get close to the real weights)
        Eigen::MatrixXd weights = randomDenseMatrix(1, N_WEIGHTS);

        // Write all data to memory
        writeMatrixState(memory, OUTPUTS_KEY, outputs);
        writeMatrixState(memory, INPUTS_KEY, inputs);
        writeMatrixState(memory, WEIGHTS_KEY, weights);

        // Also write real weights for safe keeping
        writeMatrixState(memory, REAL_WEIGHTS_KEY, realWeights);

        // Begin first epoch
        initCounter(memory, EPOCH_COUNT_KEY);
        uint8_t epochInput[1] = {0};
        memory->chainFunction("sgd_epoch", epochInput, 1);

        return 0;
    }
}