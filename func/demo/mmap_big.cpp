#include "faasm/faasm.h"

#include <sys/mman.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <errno.h>

FAASM_MAIN_FUNC() {
    // Make sure we can allocate more than 1GB of memory (i.e. >16384 pages)
    size_t nPages = 18000;
    size_t chunkSize = 1000;
    size_t nChunks = nPages / chunkSize;
    size_t chunkLen = (chunkSize * 64 * 1024);

    void **regions = new void*[nChunks];
    for(int i = 0; i < nChunks; i++) {
        void *p = mmap(nullptr, chunkLen, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (p == MAP_FAILED) {
            printf("ERROR - mmap failed: %s\n", strerror(errno));
            return 1;
        }

        regions[i] = p;
    }

    for(int i = nChunks - 1; i >= 0; i--) {
        void *p = regions[i];
        int res = munmap(p, chunkLen);
        if(res != 0) {
            printf("ERROR - munmap failed: %s\n", strerror(errno));
            return 1;
        }
    }

    printf("Big mmap succeeded\n");
    return 0;
}