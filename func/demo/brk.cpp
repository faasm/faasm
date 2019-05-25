#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    int pageSize = 65536;
    void *brkInitial = sbrk(0);
    printf("Initial = %i\n", brkInitial);

    void* newRegion = sbrk(pageSize);
    printf("New = %i\n", newRegion);

    if(brkInitial != newRegion) {
        printf("sbrk not behaving as expected\n");
        return 1;
    }

    void* newRegionB = sbrk(pageSize);
    printf("New = %i\n", newRegionB);

    int diff = ((int*) newRegionB - (int*)newRegion) * sizeof(int);
    if(diff != pageSize) {
        printf("sbrk region not size as expected (expected %i but was %i)\n", pageSize, diff);
        return 1;
    }

    return 0;
}