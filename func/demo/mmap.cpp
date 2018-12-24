#include "faasm/faasm.h"

#include <sys/mman.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        size_t memLen = 196608;

        uint8_t outputBuf[1];

        for (int i = 0; i < 10; i++) {
            // Map some memory
            void *memPtr = mmap(nullptr, memLen, 3, 34, -1, 0);

            // Now unmap that memory
            munmap(memPtr, memLen);

            // Map again
            void *memPtr2 = mmap(nullptr, memLen, 3, 34, -1, 0);

            // Check we get the same pointer
            if (memPtr != memPtr2) {
                printf("Pointers don't match %p %p\n", memPtr, memPtr2);
                outputBuf[0] = 0;
                memory->setOutput(outputBuf, 1);
                return 0;
            }
        }

        printf("Pointers all match\n");
        outputBuf[0] = 1;
        memory->setOutput(outputBuf, 1);

        return 0;
    }
}