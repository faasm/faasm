#include <faasmc/faasm.h>
#include <stdio.h>

FAASM_MAIN_FUNC() {
    printf("Hello from C!\n");
    return 0;
}
