#include "sys/mman.h"
#include <stdio.h>
#include <cstring>
#include <errno.h>
#include <unistd.h>

// Script to determine the  virtual memory limit on the system

int main() {
    float targetTb = 130.0;

    long kibiByte = 1024;
    long tibiByte = kibiByte * kibiByte * kibiByte * kibiByte;

    long long targetBytes = (long long) targetTb * tibiByte;
    int nLoops = 100000;

    long numBytes = targetBytes / nLoops;
    printf("%i loops mmapping %li bytes\n", nLoops, numBytes);

    for (int i = 0; i < nLoops; i++) {
        void *result = mmap(nullptr, numBytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
        if (result == MAP_FAILED) {
            long byteLimit = i * numBytes;
            long tbLimit = byteLimit / tibiByte;
            printf("Failed mapping on loop %i (%s)\n", i, strerror(errno));
            printf("Limit = %liTiB (%liB)\n", tbLimit, byteLimit);

            usleep(10*1000*1000);
            return 1;
        }
    }

    printf("Success over %i loops\n", nLoops);
    return 0;
}