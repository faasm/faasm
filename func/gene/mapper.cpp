#include <faasm/faasm.h>
#include <faasm/input.h>

#include <stdio.h>
#include <string>

// Index chunks
#define N_CHUNKS 1
static int indexChunks[N_CHUNKS] = {24};

/*
 * This function fans out the mapping for a given chunk of reads and awaits the results.
 */
FAASM_MAIN_FUNC() {
    std::string stringInput = faasm::getStringInput("");
    if (stringInput.empty()) {
        printf("Must provide function with read chunk index as input.\n");
        return 1;
    }

    int readIdx = std::stoi(stringInput);
    printf("Starting mapper for read index %i\n", readIdx);

    // Dispatch a call to the mapper for each index chunk and this read chunk
    // NOTE - the range of mapper_index functions is 1-based
    unsigned int callIds[N_CHUNKS];
    for (int i = 0; i < N_CHUNKS; i++) {
        // Prepare function input
        int chunkIdx = indexChunks[i];
        int input[2] = {readIdx, chunkIdx};
        auto inputBytes = (unsigned char *) input;

        // Dispatch the function
        std::string funcName = "mapper_index" + std::to_string(chunkIdx);
        unsigned int callId = faasmChainFunctionInput(funcName.c_str(), inputBytes, 2 * sizeof(int));
        printf("Chained call %u\n", callId);
        callIds[i] = callId;
    }

    // Await all finishing
    for (int i = 0; i < N_CHUNKS; i++) {
        faasmAwaitCall(callIds[i]);
    }

    printf("Finished mapping read index %i.\n", readIdx);

    return 0;
}
