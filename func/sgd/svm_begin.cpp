#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"
#include "faasm/matrix.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        long inputSize = memory->getInputSize();
        int nBatches;
        if(inputSize == 0) {
            nBatches = 3;
        } else {
            auto inputBuffer = new uint8_t[inputSize];
            memory->getInput(inputBuffer, inputSize);
            char* inputString = reinterpret_cast<char*>(inputBuffer);
            nBatches = std::stoi(inputString);
        }

        // Prepare params
        printf("Setting up SVM params\n");
        int epochs = 20;
        SgdParams p = setUpReutersParams(memory, nBatches, epochs);

        // Initialise weights
        printf("Initialising weights\n");
        Eigen::MatrixXd weights = zeroMatrix(1, p.nWeights);
        writeMatrixToState(memory, WEIGHTS_KEY, weights, p.fullAsync);

        // Zero the losses
        printf("Zeroing losses\n");
        zeroLosses(memory, p);

        // If running full async, we need to make sure the inputs and outputs are loaded fully into memory
        if(p.fullAsync) {
            printf("Loading inputs into local memory\n");
            readSparseMatrixFromState(memory, INPUTS_KEY, false);

            printf("Loading outputs into local memory\n");
            readMatrixFromState(memory, OUTPUTS_KEY, p.nTrain, 1, false);

            printf("Loading feature counts into local memory\n");
            size_t nFeatureCountBytes = p.nWeights * sizeof(int);
            memory->readState(FEATURE_COUNTS_KEY, nFeatureCountBytes, false);
        }

        // Begin first epoch
        printf("Chaining epoch call\n");
        initCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}