#include "faasm/faasm.h"
#include "faasm/counter.h"

/**
 * Function for load-testing with a state operation
 */
namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *key = "state_op";

        incrementCounter(memory, key);

        return 0;
    }
}