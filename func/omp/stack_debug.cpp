#include <faasm/faasm.h>
#include <faasmp/faasmp.h>
#include <omp.h>
#include <stdio.h>

/**
 * This function checks that OMP threads each have their own private stack.
 * The inner loop will always yield the same wasm offsets to refer to the
 * stack variables (as the code is exactly the same), but the runtime must
 * map these onto _separate_ stacks behind the scenes, rather than map them
 * all onto the same memory locations.
 *
 * If this isn't done properly, the function will fail (although what a
 * failure will look like specifically is unpredictable).
 */

FAASM_MAIN_FUNC()
{
    // The end result should be each of the thread numbers written into the
    // relevant slots
    int outputs[4] = { -2, -2, -2, -2 };
    int expected[4] = { 0, 1, 2, 3 };

// Run the parallel loop which should use the runtime to copy the thread ID from
// one stack location to the other.
#pragma omp parallel num_threads(4) default(none) shared(outputs)
    {
        // Keep the thread num in a variable that should be untouched by other
        // threads
        int threadNum = omp_get_thread_num();

        // These two stack variables will have the same offsets for every
        // thread, hence passing them out to the runtime to be written will fail
        // if the mapping is not done properly.
        int a = threadNum;
        int b = -1;
        __faasmp_debug_copy(&a, &b);

        // Record the result
        outputs[threadNum] = b;
    }

    // Check the result
    bool failed = false;
    for (int i = 0; i < 4; i++) {
        if (outputs[i] != expected[i]) {
            failed = true;
        }
    }

    printf("Stack check: {%i, %i, %i, %i}\n",
           outputs[0],
           outputs[1],
           outputs[2],
           outputs[3]);

    return failed ? 1 : 0;
}