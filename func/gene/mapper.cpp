#include <faasm/faasm.h>
#include <faasm/input.h>

#include <stdio.h>
#include <string>

// Hard-coded for human genome split into chromosomes for now
// TEMPORARY - hack to be low
//#define N_INDEX_CHUNKS 24
#define N_INDEX_CHUNKS 2

/*
 * This function fans out the mapping for a given chunk of reads and awaits the results.
 */
FAASM_MAIN_FUNC() {
    std::string stringInput = faasm::getStringInput("");
    if (stringInput == "") {
        printf("Must provide function with read chunk index as input.\n");
        return 1;
    }

    int readIdx = std::stoi(stringInput);
    printf("Starting mapper for read index %i\n", readIdx);

    // Dispatch a call to the mapper for each index chunk and this read chunk
    // NOTE - the range of mapper_index functions is 1-based
    unsigned int callIds[N_INDEX_CHUNKS];
    for (int i = 1; i <= N_INDEX_CHUNKS; i++) {
        int input[2] = {readIdx, i};
        auto inputBytes = (unsigned char *) input;

        std::string funcName = "mapper_index" + std::to_string(i);
        unsigned int callId = faasmChainFunctionInput(funcName.c_str(), inputBytes, 2 * sizeof(int));
        printf("Chained call %u\n", callId);
        callIds[i] = callId;
    }

    // Await all finishing
    for (int i = 1; i <= N_INDEX_CHUNKS; i++) {
        faasmAwaitCall(callIds[i]);
    }

    // Read the lengths of all the outputs
    const char *lengthsKey = "output_lengths";
    size_t lengthsSize = N_INDEX_CHUNKS * sizeof(int);
    int lengthsArray[N_INDEX_CHUNKS];
    faasmReadState(lengthsKey, reinterpret_cast<uint8_t *>(lengthsArray), lengthsSize);

    // Read in results for this read chunk and write them to a temporary file
    std::string tempFilePath = "/tmp/output_read_" + std::to_string(readIdx);
    FILE *fp = fopen(tempFilePath.c_str(), "ab+");
    for (int i = 1; i <= N_INDEX_CHUNKS; i++) {
        // Build key as created by mapper function
        std::string key = "map_out_" + std::to_string(readIdx) + "_" + std::to_string(i);

        // Load the output for this chunk
        size_t thisLength = (size_t) lengthsArray[i];
        uint8_t *resultBuf = new uint8_t[thisLength];
        faasmReadState(key.c_str(), resultBuf, thisLength);

        // Append to the output file
        fwrite(resultBuf, 1, thisLength, fp);
        delete[] resultBuf;
    }

    // Close the open file
    fclose(fp);

    // Write the file to state
    std::string resultKey = "output_read_" + std::to_string(readIdx);
    faasmWriteStateFromFile(resultKey.c_str(), tempFilePath.c_str());

    return 0;
}
