#include "faasm/faasm.h"
#include "faasm/counter.h"

/**
 * Function for load-testing with a state operation
 */
FAASM_MAIN_FUNC() {
    const char *key = "state_op";

    faasm::incrementCounter(key, false);

    return 0;
}