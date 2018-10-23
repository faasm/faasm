#include "faasm.h"
#include "matrix.h"
#include "random.h"

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

        int nTrain = 100; // Number of columns
        int nFactors = 10; // Number of rows

        // Create fake training data as dot product of matrix and weight vector
        const Eigen::MatrixXd &weights = randomSparseMatrix(1, nFactors);
        const Eigen::MatrixXd &factor = randomSparseMatrix(nFactors, nTrain);

        const Eigen::MatrixXd training = weights * factor;
        
        // Write all data to memory
        writeMatrixState(memory, trainingKey, training);
        writeMatrixState(memory, weightsKey, weights);
        writeMatrixState(memory, factorsKey, factor);

        // Get indices of training examples
        long trainIdx[nTrain];
        for(long i = 0; i < nTrain; i++) {
            trainIdx[i] = i;
        }

        // Shuffle
        faasm::shuffle(trainIdx, nTrain);

        int nWorkers = 10;
        int batchSize = 10;
        for(int w = 0; w < nWorkers; w++) {
            long batch[batchSize];
            std::copy(&trainIdx[w*batchSize], &trainIdx[w*batchSize + batchSize], batch);

            int nBytes = batchSize * sizeof(int);
            uint8_t *batchBytes = reinterpret_cast<uint8_t *>(&batch[0]);

            memory->chainFunction("sgd_epoch", batchBytes, nBytes);
        }

        return 0;
    }
}