#include "faasm/faasm.h"

#include <stdio.h>

namespace faasm {

    int exec(FaasmMemory *memory) {
        // Test reading a file
        FILE *fp = fopen("include/python3.7/abstract.h", "r");

        char buff[2056];
        fgets(buff, 2056, (FILE *) fp);
        printf("READ: %s\n", buff);

        return 0;
    }
}
