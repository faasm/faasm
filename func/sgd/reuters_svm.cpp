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

#define REUTERS_LEARNING_RATE 0.1
#define REUTERS_LEARNING_DECAY 0.8
#define REUTERS_N_FEATURES 47236
#define REUTERS_N_EXAMPLES 781265


SgdParams setUpReutersParams(int nBatches, int syncInterval, int epochs) {
    // Set up reuters params
    SgdParams p;
    p.nWeights = REUTERS_N_FEATURES;
    p.nTrain = REUTERS_N_EXAMPLES;
    p.learningRate = REUTERS_LEARNING_RATE;
    p.learningDecay = REUTERS_LEARNING_DECAY;
    p.nEpochs = epochs;
    p.mu = 1.0;

    // Round up number of batches
    p.nBatches = nBatches;
    p.batchSize = (REUTERS_N_EXAMPLES + nBatches - 1) / nBatches;

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

#ifdef __wasm__
#else
    setEmulatorUser("sgd");
#endif

    long inputSize = faasmGetInputSize();
    if (inputSize == 0) {
        const char *message = "Must provide two ints for input, workers and sync interval";
        setStringOutput(message);
        printf("%s\n", message);
        return 1;
    }

    const char *input = faasm::getStringInput("");
    printf("SVM input: %s\n", input);
    int *intInput = faasm::parseStringToIntArray(input, 2);

    int nWorkers = intInput[0];
    int syncInterval = intInput[1];
    int epochs = 20;
    printf("SVM running %i epochs with %i workers and sync interval %i \n", epochs, nWorkers, syncInterval);

    // Prepare params
    printf("Writing SVM params to state\n");
    faasm::SgdParams p = setUpReutersParams(nWorkers, syncInterval, epochs);
    writeParamsToState(PARAMS_KEY, p, true);

    // Initialise weights and mask
    printf("Initialising weights with zeros\n");
    Eigen::MatrixXd zeros = zeroMatrix(1, p.nWeights);
    writeMatrixToState(WEIGHTS_KEY, zeros, true);
    writeMatrixToState(MASK_KEY, zeros, true);

    // Prepare string for output
    std::string output = "\n";
    double tsZero = 0;

    // Run epochs and workers in a loop
    for (int thisEpoch = 0; thisEpoch < epochs; thisEpoch++) {
        printf("Running SVM epoch %i\n", thisEpoch);

        // Zero the errors for this epoch
        faasm::zeroErrors(p, true);

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
            if (res != 0) {
                printf("Chained call %i failed\n", res);
                return 1;
            }
        }

        // Decay learning rate (apparently hogwild doesn't actually do this although it takes in the param)
        // p.learningRate = p.learningRate * p.learningDecay;
        // writeParamsToState(PARAMS_KEY, p);

        printf("Calculating epoch loss for epoch %i\n", thisEpoch);
        // Rebase timestamp
        double ts = faasm::getSecondsSinceEpoch();
        if (tsZero == 0) {
            tsZero = ts;
        }
        ts -= tsZero;

        // Read loss (always pull)
        double loss = faasm::readRootMeanSquaredError(p, true);

        output += std::to_string(ts) + " " + std::to_string(loss) + "\n";

        printf("EPOCH %i (time = %.2f, loss = %.2f)\n", thisEpoch, ts, loss);
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
            reinterpret_cast<uint8_t *>(inputBuffer),
            inputSize
    );

    int batchNumber = inputBuffer[0];
    int startIdx = inputBuffer[1];
    int endIdx = inputBuffer[2];
    int epoch = inputBuffer[3];

    printf("SGD step: %i %i %i %i\n", batchNumber, startIdx, endIdx, epoch);

    // Load params
    SgdParams sgdParams = readParamsFromState(PARAMS_KEY, false);

    // Perform updates
    hingeLossWeightUpdate(sgdParams, epoch, batchNumber, startIdx, endIdx);

    return 0;
}
