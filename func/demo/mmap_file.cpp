#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/**
 * Map a file into memory
 */
FAASM_MAIN_FUNC()
{
    const char* fileName = "/etc/hosts";

    int fd = open(fileName, O_RDONLY);
    if (fd == -1) {
        printf("Failed to open file %s\n", fileName);
        return 1;
    }

    // Read in the full file
    struct stat sb
    {};
    fstat(fd, &sb);
    size_t bufferSize = sb.st_size;
    auto mmappedPtr =
      (char*)mmap(nullptr, bufferSize, PROT_READ, MAP_SHARED, fd, 0);

    // Check length is as expected
    size_t expectedSize = 20;
    if (expectedSize != bufferSize) {
        printf("File at %s does not have expected length (%lu != %lu)\n",
               fileName,
               bufferSize,
               expectedSize);
        printf("Contents: \n%s\n", mmappedPtr);
        return 1;
    }

    // Check the first few bytes
    int checkLength = 9;
    const char* expected = "127.0.0.1";

    if (strncmp(expected, mmappedPtr, checkLength) != 0) {
        printf(
          "Expected and actual don't match (%s != %s)\n", expected, mmappedPtr);
        return 1;
    }

    printf(
      "Expected file and actual file match (%s == %s)\n", expected, mmappedPtr);
    return 0;
}
