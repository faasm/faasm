#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    int pageSize = 65536;
    void *brkInitial = sbrk(0);

#ifdef __wasm__
    printf("Initial = %i\n", brkInitial);
#endif

    void* newRegion = sbrk(pageSize);

#ifdef __wasm__
    printf("New = %i\n", newRegion);
#endif

    if(brkInitial != newRegion) {
        printf("sbrk not behaving as expected\n");
        return 1;
    }

    void* newRegionB = sbrk(pageSize);
#ifdef __wasm__
    printf("New = %i\n", newRegionB);
#endif

    int diff = ((int*) newRegionB - (int*)newRegion) * sizeof(int);
    if(diff != pageSize) {
        printf("sbrk region not size as expected (expected %i but was %i)\n", pageSize, diff);
        return 1;
    }

    return 0;
}