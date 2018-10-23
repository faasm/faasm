#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        char *name = (char *) memory->getInput();
        printf("Running SGD %s\n", name);

        size_t nameLen = strlen(name);
        char trainingKey[nameLen + 8];
        char weightsKey[nameLen + 8];
        char factorsKey[nameLen + 8];

        sprintf(trainingKey, "%s_train", name);
        sprintf(weightsKey, "%s_weights", name);
        sprintf(factorsKey, "%s_factors", name);

        int rows = 100;
        int cols = 10;

        // Create fake training data as dot product of matrix and weight vector
        const Eigen::MatrixXd &factor = randomSparseMatrix(rows, cols);
        const Eigen::MatrixXd &weights = randomSparseMatrix(cols, 1);
        const Eigen::MatrixXd training = factor * weights;
        
        // Write all data to memory
        writeMatrixState(memory, trainingKey, training);
        writeMatrixState(memory, weightsKey, weights);
        writeMatrixState(memory, factorsKey, factor);

        return 0;
    }
}