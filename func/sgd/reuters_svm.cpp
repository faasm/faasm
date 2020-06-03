#include <faasm/faasm.h>
#include <faasm/sgd.h>
#include <faasm/input.h>
#include <faasm/matrix.h>
#include <faasm/time.h>

#include <stdio.h>

using namespace faasm;

// Params specific to Reuters dataset
#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265
#define REUTERS_N_EXAMPLES_MICRO 128


SgdParams setUpReutersParams(int nExamples, int nBatches, int syncInterval, int epochs) {
    // Set up reuters params
    SgdParams p;
    p.nWeights = REUTERS_N_FEATURES;
    p.nTrain = nExamples;
    p.learningRate = REUTERS_LEARNING_RATE;
    p.learningDecay = REUTERS_LEARNING_DECAY;
    p.nEpochs = epochs;
    p.mu = 1.0;

    // Round up batch size so that we definitely cover all the examples with _max_
    // these workers. Note that if nBatches is big relative to nExamples, we may
    // not be able to divide the work equally across all workers
    p.nBatches = nBatches;
    p.batchSize = (nExamples + nBatches - 1) / nBatches;

    // Note that the sync interval determines how often workers will
    // sync with the remote storage. There are just under 60 million updates
    // to be performed in each epoch, and a possible 47k features on which
    // these updates can occur. With lots of colocated workers these syncs
    // can be relatively infrequent, but with lots of distributed workers the
    // syncs need to be more frequent.
    //
    // Sync interval of -1 means no syncing
    p.syncInterval = syncInterval;

    return p;
}


FAASM_MAIN_FUNC() {
    long inputSize = faasmGetInputSize();
    if (inputSize == 0) {
        const char *message = "Must provide two ints for input, workers and sync interval";
        setStringOutput(message);
        printf("%s\n", message);
        return 1;
    }

    const char *input = faasm::getStringInput("");
    int *intInput = faasm::parseStringToIntArray(input, 3);

    int nWorkers = intInput[0];
    int syncInterval = intInput[1];
    int microMode = intInput[2];

    int nExamples = REUTERS_N_EXAMPLES;
    if (microMode > 0) {
        nExamples = REUTERS_N_EXAMPLES_MICRO;
    }

    // Prepare params
    int epochs;
    size_t paramsSize = faasmReadStateSize(PARAMS_KEY);
    faasm::SgdParams p;
    if (paramsSize == 0) {
        printf("Writing SVM params to state (%i examples)\n", nExamples);
        epochs = 20;
        p = setUpReutersParams(nExamples, nWorkers, syncInterval, epochs);
        writeParamsToState(PARAMS_KEY, p, true);
    } else {
        printf("Using SVM params already in state\n");
        p = readParamsFromState(PARAMS_KEY, true);
        epochs = p.nEpochs;
    }

    printf("SVM running %i epochs with %i workers and sync interval %i \n", epochs, nWorkers, syncInterval);

    // Initialise weights and mask
    printf("Initialising weights with zeros\n");
    Eigen::MatrixXd zeros = zeroMatrix(1, p.nWeights);
    writeMatrixToState(WEIGHTS_KEY, zeros, true);

    // Prepare string for output
    auto losses = new double[epochs];
    auto timestamps = new double[epochs];

    // Run epochs and workers in a loop
    double tsZero = 0;
    for (int thisEpoch = 0; thisEpoch < epochs; thisEpoch++) {
        printf("Running SVM epoch %i\n", thisEpoch);

        // Clear the errors
        faasmClearAppendedState(ERRORS_KEY);

        // Shuffle start indices for each batch
        int *batchNumbers = faasm::randomIntRange(nWorkers);

        // Run workers in a loop
        auto workerCallIds = new unsigned int[nWorkers];
        for (int w = 0; w < nWorkers; w++) {
            int startIdx = batchNumbers[w] * p.batchSize;

            // Make sure we don't overshoot
            int endIdx = std::min(startIdx + p.batchSize, p.nTrain - 1);

            // Skip this worker if it would overshoot
            if (startIdx >= endIdx) {
                printf("Skipping worker %i, not enough work with batch size %i\n", w, p.batchSize);
                continue;
            }

            // Chain the call
            int inputData[4] = {w, startIdx, endIdx, thisEpoch};
            unsigned int workerCallId = faasmChainThisInput(
                    1,
                    BYTES(inputData),
                    4 * sizeof(int)
            );
            printf("Worker %i [%i-%i] with ID %i\n", w, startIdx, endIdx, workerCallId);
            workerCallIds[w] = workerCallId;
        }

        // Wait for all workers to finish
        for (int w = 0; w < nWorkers; w++) {
            unsigned int res = faasmAwaitCall(workerCallIds[w]);
            if (res != 0) {
                printf("Chained call %i failed\n", res);
                return 1;
            }
        }

        // Decay learning rate (apparently hogwild doesn't actually do this although it takes in the param)
        // p.learningRate = p.learningRate * p.learningDecay;
        // writeParamsToState(PARAMS_KEY, p);

        // Rebase timestamp
        double ts = faasm::getSecondsSinceEpoch();
        if (tsZero == 0) {
            tsZero = ts;
        }
        ts -= tsZero;

        // Read loss
        double loss = faasm::readRootMeanSquaredError(p);

        printf("EPOCH %i (time = %.2f, loss = %.2f)\n", thisEpoch, ts, loss);

        losses[thisEpoch] = loss;
        timestamps[thisEpoch] = ts;
    }

    // Once finished, build a string of the outputs
    size_t outputLen = 0;
    int charWidth = 20;
    auto output = new char[charWidth * epochs];
    for (int e = 0; e < epochs; e++) {
        outputLen += sprintf(output + outputLen, "\n%.4f %.4f\n", timestamps[e], losses[e]);
    }

    // Force null terminator
    output[outputLen] = '\0';

    faasm::setStringOutput(output);

    return 0;
}

FAASM_FUNC(step, 1) {
    // Read in input
    long inputSize = 4 * sizeof(int);
    auto inputBuffer = new int[4];
    faasmGetInput(BYTES(inputBuffer), inputSize);

    int batchNumber = inputBuffer[0];
    int startIdx = inputBuffer[1];
    int endIdx = inputBuffer[2];
    int epoch = inputBuffer[3];

    printf("SGD step: %i %i %i %i\n", batchNumber, startIdx, endIdx, epoch);

    // Load params
    SgdParams sgdParams = readParamsFromState(PARAMS_KEY, false);

    // Perform updates
    hingeLossWeightUpdate(sgdParams, startIdx, endIdx);

    return 0;
}
