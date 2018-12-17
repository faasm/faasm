#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Set up reuters params
        SgdParams p;
        p.lossType = HINGE;
        p.nWeights = REUTERS_N_FEATURES;
        p.nTrain = REUTERS_N_EXAMPLES;
        p.learningRate = REUTERS_LEARNING_RATE;

        // Largest factors of number of training examples (111740) are:
        // 1, 2, 4, 5, 10, 20, 37, 74, 148, 151, 185, 302, 370, 604, 740, 755,
        // 1510, 3020, 5587, 11174, 22348, 27935, 55870
        //
        // Batch size should be small enough that batches don't overwrite each other too often,
        // but large enough that we aren't doing too many function calls.
        // This function allows passing the batch size in as input.
        long inputSize = memory->getInputSize();
        int batchSize;
        if(inputSize == 0) {
            batchSize = 604;
        } else {
            auto inputBuffer = new uint8_t[inputSize];
            memory->getInput(inputBuffer, inputSize);
            char* inputString = reinterpret_cast<char*>(inputBuffer);
            batchSize = std::stoi(inputString);
        }

        printf("Starting SVM with batch size %i\n", batchSize);
        p.nBatches = p.nTrain / batchSize;
        p.nEpochs = 60;

        writeParamsToState(memory, PARAMS_KEY, p);
        memory->forcePushState(PARAMS_KEY);

        // Initialise weights
        Eigen::MatrixXd weights = randomDenseMatrix(1, p.nWeights);
        writeMatrixToState(memory, WEIGHTS_KEY, weights);
        memory->forcePushState(PARAMS_KEY);

        // Zero the losses
        zeroLosses(memory, p);
        memory->forcePushState(LOSSES_KEY);

        // Begin first epoch
        initCounter(memory, EPOCH_COUNT_KEY);
        memory->chainFunction("sgd_epoch");
        memory->forcePushState(EPOCH_COUNT_KEY);

        return 0;
    }
}