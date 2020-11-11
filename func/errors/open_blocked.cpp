#include "faasm/faasm.h"

#include <stdint.h>
#include <stdio.h>

/** Attempts to open a blocked file */
int main(int argc, char* argv[])
{
    FILE* fp;
    fp = fopen("/etc/shadow", "r");

    if (fp == NULL) {
        return 1;
    }

    char buffer[100];

    // Read in
    size_t i = fread(buffer, 10, 1, fp);
    printf("Read %li\n", i);
    printf("%s\n", buffer);

    fclose(fp);

    return 0;
}
