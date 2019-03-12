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

    // Map file into memory copy-on-write
    void *ptrA = mmap(nullptr, len, PROT_WRITE, MAP_PRIVATE, fd, 0);
    void *ptrB = mmap(nullptr, len, PROT_WRITE, MAP_PRIVATE, fd, 0);

    // Get as int pointers
    int *otherA = reinterpret_cast<int*>(ptrA);
    int *otherB = reinterpret_cast<int*>(ptrB);

    printf("----------------\n");
    _printArray(otherA);
    _printArray(otherB);

    // Write a single element of A
    otherA[2] = 9;

    printf("----------------\n");
    _printArray(otherA);
    _printArray(otherB);

    // Write a single element of B
    otherB[1] = 88;

    printf("----------------\n");
    _printArray(otherA);
    _printArray(otherB);
}
