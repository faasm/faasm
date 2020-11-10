#include "faasm/counter.h"
#include "faasm/faasm.h"

/**
 * Function for load-testing with a state operation
 */
int main(int argc, char* argv[])
{
    const char* key = "state_op";

    faasm::incrementCounter(key);
    faasmPushState(key);

    return 0;
}
