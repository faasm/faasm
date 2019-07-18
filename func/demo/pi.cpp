#include <faasm/faasm.h>
#include <faasm/input.h>
#include <faasm/counter.h>
#include <faasm/state.h>
#include <faasm/random.h>

#include <vector>
#include <stdio.h>

#define N_GUESSES 1000000
#define CHUNK_SIZE_KEY "chunk_size"
#define COUNT_KEY "count"

/**
 * Parallel estimation of pi
 */
FAASM_MAIN_FUNC() {
    int nWorkers = faasm::getIntInput(4);

    // Write chunk size to state
    int chunkSize = N_GUESSES / nWorkers;
    faasm::writeIntState(CHUNK_SIZE_KEY, chunkSize);

    // Set count to zero
    faasm::writeIntState(COUNT_KEY, 0);

    // Dispatch chained calls in a loop
    std::vector<int> callIds;
    for (int i = 0; i < nWorkers; i++) {
        int callId = faasmChainThis(1);
        callIds.push_back(callId);
    }

    // Wait for calls to finish
    for (int callId : callIds) {
        faasmAwaitCall(callId);
    }

    int finalCount = faasm::readIntState(COUNT_KEY);
    float piEstimate = 4*((float)finalCount / (N_GUESSES));

    std::string output = "Pi estimate: " + std::to_string(piEstimate) + "\n";
    printf("%s", output.c_str());
    faasm::setStringOutput(output.c_str());

    return 0;
}

FAASM_FUNC(piStep, 1) {
    int chunkSize = faasm::readIntState(CHUNK_SIZE_KEY);

    int count = 0;
    for (int i = 0; i < chunkSize; i++) {
        // Two random points
        double x = faasm::randomFloat();
        double y = faasm::randomFloat();

        // Check if within unit circle
        if (x * x + y * y <= 1.0) {
            count = count + 1;
        }
    }

    faasm::incrementCounter(COUNT_KEY, count, false);

    return 0;
}
