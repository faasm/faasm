#include "faasm/faasm.h"

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/** Attempts to open a blocked file */
namespace faasm {
    int exec(FaasmMemory *memory) {
        FILE *fp = fopen("/etc/passwd", "r");
        printf("FP: %p\n", fp);
        printf("fopen done\n");

        printf("errno: %s\n", strerror(errno));
//        char buffer[100];

        // Read in
//        size_t i = fread(buffer, 10, 1, fp);
//        printf("Read %li\n", i);
//        printf("%s\n", buffer);

        fclose(fp);

        return 0;
    }
}
