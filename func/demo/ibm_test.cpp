#include <faasm/faasm.h>
#include <stdio.h>

/**
 * This is just needed for testing things with IBM Cloud functions
 */
FAASM_MAIN_FUNC() {
    printf("IBM test\n");
    return 0;
}
