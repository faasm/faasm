#include "faasm/faasm.h"

#include <malloc.h>
#include <string.h>
#include <string>
#include <sys/mman.h>

/*
 * Checks that calloc is working.
 *
 * Originally created to try to replicate an issue from CPython
 */
int main(int argc, char* argv[])
{
    int callocSize = 100;
    int n = 1000;
    char* callocPtrs[n];
    char* mmapPtrs[n];
    int mmapLen = 20;

    // Use calloc to allocate lots of smaller memory regions
    for (int i = 0; i < 1000; i++) {
        char* callocPtr = (char*)calloc(1, callocSize);
        std::string s = "number ";
        s += std::to_string(i);
        strcpy(callocPtr, s.c_str());

        // Add in a call to mmap to deliberately fragment things
        char* mmapPtr =
          (char*)mmap(nullptr, mmapLen, PROT_WRITE, MAP_ANONYMOUS, -1, 0);
        strcpy(mmapPtr, "mmapSpace");

        callocPtrs[i] = callocPtr;
        mmapPtrs[i] = mmapPtr;
    }

    // Go back through and check everything is still intact
    for (int j = 0; j < 1000; j++) {
        char* nextPtr = callocPtrs[j];
        std::string actual(nextPtr);
        std::string expected = "number ";
        expected += std::to_string(j);

        if (expected != actual) {
            printf("Error with calloc: %s != %s\n",
                   expected.c_str(),
                   actual.c_str());
            return 1;
        }

        // Free up the space
        free(callocPtrs[j]);
        munmap(mmapPtrs[j], mmapLen);
    }

    return 0;
}
