#include <stdio.h>
#include <sgx/faasm_sgx_wamr.h>

void FAASM_MAIN(main_) {
    printf("[Info] Hello World from sgx_wamr\n");
    return;
}
