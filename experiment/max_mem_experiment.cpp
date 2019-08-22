#include "sys/mman.h"
#include <stdio.h>
#include <cstring>
#include <errno.h>


int main() {
    float targetTb = 130.0;

    long kibiByte = 1024;
    long tibiByte = kibiByte * kibiByte * kibiByte * kibiByte;

    long long targetBytes = (long long) targetTb * tibiByte;
    int nLoops = 5000;

    long numBytes = targetBytes / nLoops;
    printf("%i loops mmapping %li bytes\n", nLoops, numBytes);

    for (int i = 0; i < nLoops; i++) {
        void *result = mmap(nullptr, numBytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (result == MAP_FAILED) {
            long limit = i * numBytes;
            long tbLimit = limit / tibiByte;
            printf("Failed mapping on loop %i (%s)\nLimit = %liTiB\n", i, strerror(errno), tbLimit);
            return 1;
        }
    }

    printf("Success over %i loops\n", nLoops);
    return 0;
}