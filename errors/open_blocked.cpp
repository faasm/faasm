#include "faasm/faasm.h"

/** Attempts to open a blocked file */
namespace faasm {
    int exec(FaasmMemory *memory) {
        FILE * fp;
        fp = fopen("/etc/passwd", "r");

        char buffer[100];

        // Read in
        fread(buffer, 10, 1, fp);
        printf("%s\n", buffer);

        fclose(fp);

        return 0;
    }
}
