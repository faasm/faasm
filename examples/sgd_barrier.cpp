#include "faasm.h"
#include "matrix.h"
#include "counter.h"

namespace faasm {
    bool checkWorkers(FaasmMemory *memory) {
        int nWorkers = 10;

        // Check state for each worker
        for (int i = 0; i < nWorkers; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);

            uint8_t count = getCounter(memory, workerKey);
            if (count < 1) {
                return false;
            }
        }

        return true;
    }

    int exec(FaasmMemory *memory) {
        // Get current epoch count
        const char *epochCountKey = "epochCount";
        uint8_t thisEpoch = getCounter(memory, epochCountKey);

        // Drop out if over max
        uint8_t maxEpochs = 10;
        if (thisEpoch >= maxEpochs) {
            printf("SGD complete over %i epochs\n", thisEpoch);
            return 0;
        }

        // Resubmit if not yet finished
        bool workersFinished = checkWorkers(memory);
        if (!workersFinished) {
            // Recursive call
            uint8_t barrierInput[1] = {0};
            memory->chainFunction("sgd_barrier", barrierInput, 1);
            return 0;
        }

        // Work out difference between weights
        int nWeights = 10;
        const char *weightsKey = "weights";
        MatrixXd weights = faasm::readMatrixFromState(memory, weightsKey, 1, nWeights);

        const char *realWeightsKey = "realWeights";
        MatrixXd realWeights = faasm::readMatrixFromState(memory, realWeightsKey, 1, nWeights);

        // Calculate MSE
        MatrixXd diff = realWeights - weights;
        double mse = 0;
        for(int r = 0; r < diff.rows(); r++) {
            for(int c = 0; c < diff.cols(); c++) {
                double e = diff.coeff(r, c);
                mse += pow(e, 2);
            }
        }
        mse /= nWeights;

        // Increment epoch counter
        incrementCounter(memory, epochCountKey);
        int nextEpoch = thisEpoch + 1;
        printf("Starting epoch %i (MSE = %f)\n", nextEpoch, mse);

        // Load inputs
        const char *inputsKey = "inputs";
        int nTrain = 1000;
        MatrixXd inputs = faasm::readMatrixFromState(memory, inputsKey, nWeights, nTrain);

        // Shuffle and update
        faasm::shuffleMatrixColumns(inputs);
        writeMatrixState(memory, inputsKey, inputs);

        // Kick off next epoch
        uint8_t epochInput[1] = {0};
        memory->chainFunction("sgd_epoch", epochInput, 1);

        return 0;
    }
}