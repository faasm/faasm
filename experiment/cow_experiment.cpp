#include "stdio.h"
#include "sys/mman.h"
#include <unistd.h>
#include <errno.h>
#include <thread>
#include <string.h>


void _printArray(int * arr) {
    printf("%i %i %i %i %i\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
}

int main() {
    int pageSize = getpagesize();
    printf("Page size: %i\n", pageSize);

    // Create an anonymous file
    const char* name = "foobar";
    int fd = memfd_create(name, 0);

    // Set size
    size_t len = 5 * sizeof(int);
    ftruncate(fd, len);

    // Write data
    int original[] = {1, 2, 3, 4, 5};
    write(fd, original, len);
    printf("Original:\n");
    _printArray(original);

    // Create other memories (these are like the wasm module memories
    printf("----------------\n");
    void *otherMemA = mmap(nullptr, len, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *otherMemB = mmap(nullptr, len, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    int *otherA = reinterpret_cast<int*>(otherMemA);
    int *otherB = reinterpret_cast<int*>(otherMemB);
    _printArray(otherA);
    _printArray(otherB);

    // Map the file into these locations, this will implicitly unmap the original stuff but that's what we want
    void *ptrA = mmap(otherMemA, len, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);
    void *ptrB = mmap(otherMemB, len, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);

    // Get as int pointers
    int *mappedA = reinterpret_cast<int*>(ptrA);
    int *mappedB = reinterpret_cast<int*>(ptrB);

    printf("----------------\n");
    _printArray(mappedA);
    _printArray(mappedB);

    // Write a single element of A
    mappedA[2] = 9;

    printf("----------------\n");
    _printArray(mappedA);
    _printArray(mappedB);

    // Write a single element of B
    mappedB[1] = 88;

    printf("----------------\n");
    _printArray(mappedA);
    _printArray(mappedB);

    // Now test reading from the original memory locations
    printf("----------------\n");
    _printArray(otherA);
    _printArray(otherB);
}
