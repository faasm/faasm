#include "faasm/faasm.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <errno.h>

FAASM_MAIN_FUNC() {
    // Make sure we can allocate more than 2GB of memory (i.e. >32768 pages)
    size_t nPages = 35000;
    size_t chunkSize = 1000;
    size_t nChunks = nPages / chunkSize;

    for(int i = 0; i < nChunks; i++) {
        size_t memLen = (chunkSize * 64 * 1024);
        void *p = mmap(nullptr, memLen, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (p == MAP_FAILED) {
            printf("ERROR - mmap failed: %s\n", strerror(errno));
            return 1;
        }
    }

    printf("Big mmap succeeded\n");
    return 0;
}