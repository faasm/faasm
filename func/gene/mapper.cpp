#include <stdio.h>
#include <faasm/faasm.h>
#include <faasm/input.h>

#define N_READ_CHUNKS 10
#define N_INDEX_CHUNKS 25

/*
 * This is the function that fans out the mapping for a given chunk of reads
 * and awaits the results.
 */
FAASM_FUNC(read_worker, 1) {
    int readIdx = faasm::getTypedInput<int>(-1);
    if (readIdx == -1) {
        printf("Must provide function with read chunk index as input.\n");
        return 1;
    }

    // Dispatch a worker for each index chunk
    unsigned int callIds[N_INDEX_CHUNKS];
    for (int i = 0; i < N_INDEX_CHUNKS; i++) {
//        int input[2] = {readIdx, i};
//        auto inputBytes = (unsigned char *) input;

        // TODO - chain call to gene/gem3mapper
//        unsigned int callId = faasmChain(1, inputBytes, 2 * sizeof(int));
//
//        callIds[i] = callId;
    }

    // Await all finishing
    for (unsigned int callId : callIds) {
        faasmAwaitCall(callId);
    }

    return 0;
}

/*
 * This is the entrpoint for the genomics mapper. Its job is to fan out all the
 * mapper workers which do the real work with gem3.
 */
FAASM_MAIN_FUNC() {
    unsigned int callIds[N_READ_CHUNKS];
    for (unsigned int r = 0; r < N_READ_CHUNKS; r++) {
        // Chain the reads chunk worker for this read chunk
        auto inputBytes = reinterpret_cast<unsigned char*>(&r);
        unsigned int callId = faasmChainThisInput(1, inputBytes, sizeof(int));

        callIds[r] = callId;
    }

    // Await all finishing
    for (auto callId : callIds) {
        faasmAwaitCall(callId);
    }

    // TODO - aggregation

    return 0;
}