#include "faasm.h"

/**
 * Writes the input to the output
 */
namespace faasm {
    int exec(struct FaasmMemory *memory) {
        for (int i = 0; i < MAX_INPUT_BYTES; i++) {
            memory->output[i] = memory->input[i];
        }

        printf("MEMORY IN: %s \n", memory->input);
        printf("MEMORY OUT: %s \n", memory->output);

        return 0;
    }
}
