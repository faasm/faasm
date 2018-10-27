#include "faasm.h"
#include "matrix.h"
#include "random.h"
#include "counter.h"


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
        int nWorkers = 10;
        int batchSize = 100;
        uint8_t workerStatus[] = {0};
        for (int w = 0; w < nWorkers; w++) {
            int startIdx = (w * batchSize);
            int endIdx = startIdx + batchSize -1;

            // Prepare input data for the worker
            int inputData[4] = {w, startIdx, endIdx, nWeights};
            int nBytes = 4 * sizeof(int);
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);

            // Mark this worker as having started
            char workerKey[10];
            sprintf(workerKey, "worker-%i", w);
            initCounter(memory, workerKey);

            // Call the chained function
            memory->chainFunction("sgd_update", inputBytes, nBytes);
        }

        // Record how many epochs have been
        initCounter(memory, "epochCount");

        // Dispatch the barrier function
        uint8_t barrierInput[1] = {0};
        memory->chainFunction("sgd_barrier", barrierInput, 1);

        return 0;
    }
}