#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"
#include "faasm/matrix.h"
#include "faasm/func.h"
#include "faasm/time.h"
#include "faasm/print.h"

#include <stdio.h>

/* If running a native build we need to explicitly set the user on the emulator */
#if WASM_BUILD == 1
#else
#include "emulator/emulator.h"
#endif


using namespace faasm;

void epochFinished() {
    bool fullAsync = getEnvFullAsync();
    SgdParams p = readParamsFromState(PARAMS_KEY, fullAsync);

    // Get epoch count and barrier count(starts at zero)
    int thisEpoch = getCounter(EPOCH_COUNT_KEY, p.fullAsync);

    long tsTotal = p.nEpochs * sizeof(double);
    long tsOffset = thisEpoch * sizeof(double);

    // Record the time for this epoch
    double ts = faasm::getSecondsSinceEpoch();
    auto tsBytes = reinterpret_cast<uint8_t *>(&ts);
    faasmWriteStateOffset(LOSS_TIMESTAMPS_KEY, tsTotal, tsOffset, tsBytes, sizeof(double), p.fullAsync);

    // Record the loss for this epoch
    double loss = faasm::readRootMeanSquaredError(p);
    auto lossBytes = reinterpret_cast<uint8_t *>(&loss);
    faasmWriteStateOffset(LOSSES_KEY, tsTotal, tsOffset, lossBytes, sizeof(double), p.fullAsync);

    // Decay learning rate (apparently hogwild doesn't actually do this although it takes in the param)
    // p.learningRate = p.learningRate * p.learningDecay;
    // writeParamsToState(PARAMS_KEY, p);

    // Increment epoch counter
    incrementCounter(EPOCH_COUNT_KEY, p.fullAsync);
    int nextEpoch = thisEpoch + 1;
    char *tsString = faasm::floatToStr(ts);
    char *lossString = faasm::floatToStr(loss);
    printf("Starting epoch %i (time = %s, loss = %s)\n", nextEpoch, tsString, lossString);
}

FAASM_MAIN_FUNC() {

#if WASM_BUILD == 1
#else
    setEmulatorUser("sgd");
#endif

    long inputSize = faasmGetInputSize();
    int nBatches;
    if (inputSize == 0) {
        nBatches = 2;
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

    // Run the epochs
    initCounter(EPOCH_COUNT_KEY, p.fullAsync);
    for (int i = 0; i < p.nEpochs; i++) {
        printf("Chaining epoch call\n");
        unsigned int epochCallId = faasmChainThis(1);
        faasmAwaitCall(epochCallId);

        // Epoch finished at this point
        epochFinished();
    }

    return 0;
}

FAASM_FUNC(epoch, 1) {
    // Load params
    bool fullAsync = faasm::getEnvFullAsync();
    faasm::SgdParams p = faasm::readParamsFromState(PARAMS_KEY, fullAsync);

    // Set per-epoch memory to zero
    faasm::zeroErrors(p);
    faasm::zeroFinished(p);

    // Get the epoch count
    int epoch = faasm::getCounter(EPOCH_COUNT_KEY, p.fullAsync);

    // Shuffle start indices for each batch
    int *batchNumbers = faasm::randomIntRange(p.nBatches);

    // Chain new calls to perform the work
    std::vector<int> workerCallIds;
    for (int w = 0; w < p.nBatches; w++) {
        int startIdx = batchNumbers[w] * p.batchSize;

        // Make sure we don't overshoot
        int endIdx = std::min(startIdx + p.batchSize, p.nTrain - 1);

        // Prepare input data for the worker as a string
        std::string args = std::to_string(w) + " " + std::to_string(startIdx) + " " + std::to_string(endIdx) + " " +
                           std::to_string(epoch);
        printf("Chaining sgd_step with args: %s\n", args.c_str());

        auto inputBytes = reinterpret_cast<const uint8_t *>(args.c_str());

        // Call the chained function
        unsigned int thisCallId = faasmChainThisInput(2, inputBytes, args.size());
        printf("Worker spawned with call ID %i\n", thisCallId);
        workerCallIds.push_back(thisCallId);
    }

    // Wait for all workers to finish
    for (auto callId : workerCallIds) {
        faasmAwaitCall(callId);
    }

    delete[] batchNumbers;

    return 0;
}

FAASM_FUNC(step, 2) {
    bool fullAsync = getEnvFullAsync();

    long inputSize = faasmGetInputSize();
    auto inputBuffer = new uint8_t[inputSize];
    faasmGetInput(inputBuffer, inputSize);

    const char *inputStr = reinterpret_cast<const char *>(inputBuffer);
    int *intArgs = faasm::splitStringIntoInts(inputStr, 4);

    int batchNumber = intArgs[0];
    int startIdx = intArgs[1];
    int endIdx = intArgs[2];
    int epoch = intArgs[3];

    printf("SGD step: %i %i %i %i\n", batchNumber, startIdx, endIdx, epoch);

    // Load params
    SgdParams sgdParams = readParamsFromState(PARAMS_KEY, fullAsync);

    // Perform updates
    if (sgdParams.lossType == HINGE) {
        printf("SGD hinge weight update\n");
        hingeLossWeightUpdate(sgdParams, epoch, batchNumber, startIdx, endIdx);
    } else {
        printf("SGD least squares weight update\n");
        leastSquaresWeightUpdate(sgdParams, batchNumber, startIdx, endIdx);
    }

    // Flag that this worker has finished
    printf("Writing finished flag\n");
    writeFinishedFlag(sgdParams, batchNumber);

    // If this is the last, dispatch the barrier (will have finished by now or will do soon)
    if (batchNumber == sgdParams.nBatches - 1) {
        faasmChainFunction("sgd_barrier");
    }

    delete[] intArgs;

    return 0;
}