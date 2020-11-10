#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char* argv[])
{
    size_t memLen = 196608;

    // Map some memory
    void* memPtr = mmap(nullptr, memLen, PROT_WRITE, MAP_PRIVATE, -1, 0);

    // Write something to it
    strcpy((char*)memPtr, "Hi there!");
    if (strcmp((const char*)memPtr, "Hi there!") != 0) {
        printf("ERROR - not written to mmapped region as expected\n");
        return 1;
    }

    // Now unmap that memory
    munmap(memPtr, memLen);

    // Write something to it again (should fail)
    strcpy((char*)memPtr, "Hi there!");

    printf("ERROR - should have failed by now\n");

    return 0;
}
