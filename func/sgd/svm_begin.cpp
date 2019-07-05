#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"
#include "faasm/matrix.h"

#include <stdio.h>

using namespace faasm;

FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();
    int nBatches;
    if (inputSize == 0) {
        nBatches = 200;
        printf("SVM running default %i batches \n", nBatches);
    } else {
        auto inputBuffer = new uint8_t[inputSize];
        faasmGetInput(inputBuffer, inputSize);
        char *inputString = reinterpret_cast<char *>(inputBuffer);
        nBatches = std::stoi(inputString);

        printf("SVM running %i batches as requested\n", nBatches);
    }

    // Prepare params
    printf("Setting up SVM params\n");
    int epochs = 20;
    faasm::SgdParams p = setUpReutersParams(nBatches, epochs);

    // Initialise weights
    printf("Initialising weights\n");
    Eigen::MatrixXd weights = zeroMatrix(1, p.nWeights);
    writeMatrixToState(WEIGHTS_KEY, weights, p.fullAsync);

    // Zero the losses
    printf("Zeroing losses\n");
    zeroLosses(p);

    // If running full async, we need to make sure the inputs and outputs are loaded fully into memory
    if (p.fullAsync) {
        printf("Loading inputs into local memory\n");
        readSparseMatrixFromState(INPUTS_KEY, false);

        printf("Loading outputs into local memory\n");
        readMatrixFromState(OUTPUTS_KEY, p.nTrain, 1, false);

        printf("Loading feature counts into local memory\n");
        size_t nFeatureCountBytes = p.nWeights * sizeof(int);
        faasmReadStatePtr(FEATURE_COUNTS_KEY, nFeatureCountBytes, false);
    }

    // Begin first epoch
    printf("Chaining epoch call\n");
    initCounter(EPOCH_COUNT_KEY, p.fullAsync);
    faasmChainFunction("sgd_epoch");

    return 0;
}