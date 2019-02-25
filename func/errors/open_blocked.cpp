#include "faasm/faasm.h"

#include <stdint.h>
#include <stdio.h>

/** Attempts to open a blocked file */
namespace faasm {
    int exec(FaasmMemory *memory) {
        FILE * fp;
        fp = fopen("/etc/passwd", "r");

        char buffer[100];

        // Read in
        size_t i = fread(buffer, 10, 1, fp);
        printf("Read %li\n", i);
        printf("%s\n", buffer);

        fclose(fp);

        return 0;
    }
}
