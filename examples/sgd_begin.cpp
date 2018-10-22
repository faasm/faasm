#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *name = (char *) memory->getInput();
        printf("Running SGD %s\n", name);

        int rows = 100;
        int cols = 10;

        // Create fake training data as dot product of matrix and weight vector
        const Eigen::MatrixXd &factor = randomSparseMatrix(rows, cols);
        const Eigen::VectorXd weights(cols);

        // Create training data
        const Eigen::MatrixXd training = factor * weights;
        
        // Convert training data to an array
        size_t nBytes = rows * cols * sizeof(double);
        uint8_t *serialisedTraining = faasm::matrixToBytes(training);
        
        // Persist
        memory->writeState(name, serialisedTraining, nBytes);

        // Read state again
        uint8_t buffer[nBytes];
        memory->readState(name, buffer, nBytes);

        Eigen::MatrixXd deserialised = faasm::bytesToMatrix(buffer, rows, cols);

        return 0;
    }
}