#include <faasm/faasm.h>
#include <faasm/sgd.h>
#include <faasm/input.h>
#include <faasm/matrix.h>
#include <faasm/time.h>

#include <stdio.h>

/* If running a native build we need to explicitly set the user on the emulator */
#ifdef __wasm__
#else

#include "emulator/emulator.h"

#endif

using namespace faasm;

FAASM_MAIN_FUNC() {

#ifdef __wasm__
#else
    setEmulatorUser("sgd");
#endif

    int nWorkers = faasm::getIntInput(1);
    printf("SVM running %i batches \n", nWorkers);

    // Prepare params
    printf("Setting up SVM params\n");
    int epochs = 20;
    faasm::SgdParams p = setUpReutersParams(nWorkers, epochs);

    // Initialise weights
    printf("Initialising weights with zeros\n");
    Eigen::MatrixXd weights = zeroMatrix(1, p.nWeights);
    writeMatrixToState(WEIGHTS_KEY, weights, p.fullAsync);

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

    // Prepare string for output
    std::string output;

    // Run epochs and workers in a loop
    for (int thisEpoch = 0; thisEpoch < epochs; thisEpoch++) {
        printf("Running SVM epoch %i\n", thisEpoch);

        // Zero the errors for this epoch
        faasm::zeroErrors(p);

        // Shuffle start indices for each batch
        int *batchNumbers = faasm::randomIntRange(nWorkers);

        // Run workers in a loop
        auto workerCallIds = new unsigned int[nWorkers];
        for (int w = 0; w < nWorkers; w++) {
            int startIdx = batchNumbers[w] * p.batchSize;

            // Make sure we don't overshoot
            int endIdx = std::min(startIdx + p.batchSize, p.nTrain - 1);

            // Chain the call
            int inputData[4] = {w, startIdx, endIdx, thisEpoch};
            unsigned int workerCallId = faasmChainThisInput(
                    1,
                    reinterpret_cast<uint8_t *>(inputData),
                    4 * sizeof(int)
            );
            printf("Worker spawned with call ID %i\n", workerCallId);
            workerCallIds[w] = workerCallId;
        }

        // Wait for all workers to finish
        for (int w = 0; w < nWorkers; w++) {
            unsigned int res = faasmAwaitCall(workerCallIds[w]);
            if(res != 0) {
                printf("Chained call %i failed\n", res);
                return 1;
            }
        }

        // Record the time and loss for this epoch
        printf("Calculating epoch loss for epoch %i\n", thisEpoch);
        double ts = faasm::getSecondsSinceEpoch();
        double loss = faasm::readRootMeanSquaredError(p);

        output += std::to_string(thisEpoch) + ": " + std::to_string(ts) + ": " + std::to_string(loss);
        if (thisEpoch != epochs - 1) {
            output += ",";
        }

        // Decay learning rate (apparently hogwild doesn't actually do this although it takes in the param)
        // p.learningRate = p.learningRate * p.learningDecay;
        // writeParamsToState(PARAMS_KEY, p);

        printf("Finished epoch %i (time = %.2f, loss = %.2f)\n", thisEpoch, ts, loss);
    }

    printf("Finished SVM: %s\n", output.c_str());

    // Once finished, we want to return the full output details
    faasm::setStringOutput(output.c_str());

    return 0;
}

FAASM_FUNC(step, 1) {
    // Read in input
    long inputSize = 4 * sizeof(int);
    int inputBuffer[4];
    faasmGetInput(
            reinterpret_cast< uint8_t *>(inputBuffer),
            inputSize
    );

    int batchNumber = inputBuffer[0];
    int startIdx = inputBuffer[1];
    int endIdx = inputBuffer[2];
    int epoch = inputBuffer[3];

    printf("SGD step: %i %i %i %i\n", batchNumber, startIdx, endIdx, epoch);

    // Load params
    bool fullAsync = getEnvFullAsync();
    SgdParams sgdParams = readParamsFromState(PARAMS_KEY, fullAsync);

    // Perform updates
    hingeLossWeightUpdate(sgdParams, epoch, batchNumber, startIdx, endIdx);

    return 0;
}