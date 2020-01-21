#include "stdio.h"
#include "sys/mman.h"
#include <unistd.h>
#include <string.h>
#include <iostream>

#define N_INTS     5
#define N_BYTES    5 * sizeof(int)

void _printArray(int *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        std::cout << arr[i];

        if (i != len - 1) {
            std::cout << ",";
        }
    }

    std::cout << std::endl;
}

int main() {
    int pageSize = getpagesize();
    printf("Page size: %i\n", pageSize);

    // Create an anonymous file
    const char *name = "foobar";
    int fd = memfd_create(name, 0);

    // Set size
    ftruncate(fd, N_BYTES);

    // Write data
    int original[] = {1, 2, 3, 4, 5};
    write(fd, original, N_BYTES);
    printf("Original:\n");
    _printArray(original, N_INTS);

    // Create other memories (these are like the wasm module memories)
    // Make them bigger than we actually need and not writable initially
    printf("----------------\n");
    void *otherMemA = mmap(nullptr, 2 * N_BYTES, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *otherMemB = mmap(nullptr, 2 * N_BYTES, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int *otherA = reinterpret_cast<int *>(otherMemA);
    int *otherB = reinterpret_cast<int *>(otherMemB);

    // Now allow writes on the first bits and print as they are
    mprotect(otherMemA, N_BYTES, PROT_READ | PROT_WRITE);
    mprotect(otherMemB, N_BYTES, PROT_READ | PROT_WRITE);
    _printArray(otherA, N_INTS);
    _printArray(otherB, N_INTS);

    // Map the file into these locations, this will implicitly unmap the original stuff but that's what we want
    void *ptrA = mmap(otherMemA, N_BYTES, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);
    void *ptrB = mmap(otherMemB, N_BYTES, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);

    // Get as int pointers
    int *mappedA = reinterpret_cast<int *>(ptrA);
    int *mappedB = reinterpret_cast<int *>(ptrB);

    printf("----------------\n");
    _printArray(mappedA, N_INTS);
    _printArray(mappedB, N_INTS);

    // Write a single element of A
    mappedA[2] = 9;

    printf("----------------\n");
    _printArray(mappedA, N_INTS);
    _printArray(mappedB, N_INTS);

    // Write a single element of B
    mappedB[1] = 88;

    printf("----------------\n");
    _printArray(mappedA, N_INTS);
    _printArray(mappedB, N_INTS);

    // Now test reading from the original memory locations
    printf("----------------\n");
    _printArray(otherA, N_INTS);
    _printArray(otherB, N_INTS);

    // Now grow one of the regions and write to a higher part
    mprotect(reinterpret_cast<uint8_t *>(otherMemA) + N_BYTES, PROT_WRITE, N_BYTES);
    otherA[N_INTS + 2] = 81;
    printf("----------------\n");
    _printArray(otherA, 2 * N_INTS);
    _printArray(otherB, 2 * N_INTS);


}
