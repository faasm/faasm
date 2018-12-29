#include "stdio.h"
#include "sys/mman.h"
#include <unistd.h>
#include <errno.h>


int main() {
    int pageSize = getpagesize();
    printf("Page size: %i\n", pageSize);

    // Request some memory
    size_t memSize = 4 * pageSize;
    void *mappedRegion = mmap(nullptr, memSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *original = reinterpret_cast<int *>(mappedRegion);
    *original = 55;

    printf("Mapped region: %p\n", mappedRegion);

    // Request more memory
    void *mappedRegion2 = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *mappedRegion3 = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Map shared region onto new region
    mremap(mappedRegion, 0, memSize, MREMAP_FIXED | MREMAP_MAYMOVE, mappedRegion2);
    mremap(mappedRegion, 0, memSize, MREMAP_FIXED | MREMAP_MAYMOVE, mappedRegion3);

    // Write something to original region and check it is reflected in others
    int *copy1 = reinterpret_cast<int *>(mappedRegion2);
    int *copy2 = reinterpret_cast<int *>(mappedRegion3);

    printf("%i  %i  %i\n", *original, *copy1, *copy2);

    // Unmap one region to anonymous
    munmap(mappedRegion2, memSize);
    mmap(mappedRegion2, memSize, PROT_WRITE, MAP_PRIVATE| MAP_ANONYMOUS, -1, 0);
    *copy1 = 99;

    // Update original and check value isn't passed to unmapped
    *original = 66;

    printf("%i  %i  %i\n", *original, *copy1, *copy2);
}
