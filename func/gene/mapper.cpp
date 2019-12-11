#include <faasm/faasm.h>
#include <faasm/input.h>

#include <stdio.h>
#include <string>

// Hard-coded for human genome split into chromosomes for now
#define N_INDEX_CHUNKS 25

/*
 * This function fans out the mapping for a given chunk of reads and awaits the results.
 */
FAASM_MAIN_FUNC() {
    int readIdx = faasm::getTypedInput<int>(-1);
    if (readIdx == -1) {
        printf("Must provide function with read chunk index as input.\n");
        return 1;
    }

    // Dispatch a call to the mapper for each index chunk and this read chunk
    unsigned int callIds[N_INDEX_CHUNKS];
    for (int i = 0; i < N_INDEX_CHUNKS; i++) {
        int input[2] = {readIdx, i};
        auto inputBytes = (unsigned char *) input;

        std::string funcName = "mapper_index" + std::to_string(i);
        unsigned int callId = faasmChainFunctionInput(funcName.c_str(), inputBytes, 2 * sizeof(int));
        callIds[i] = callId;
    }

    // Await all finishing
    for (unsigned int callId : callIds) {
        faasmAwaitCall(callId);
    }

    // Read in results for this read chunk in order
    for(int i = 0; i < N_INDEX_CHUNKS; i++) {
        // Build key as created by mapper function
        std::string key = "map_out_" + std::to_string(readIdx) + "_" + std::to_string(i);

        // Load output and append
        
    }

    return 0;
}
