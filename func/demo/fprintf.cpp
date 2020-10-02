#include "faasm/faasm.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
    fprintf(stderr, "testing 123\n");
    fputs("testing 345\n", stderr);

    fputc('a', stderr);
    fputc('b', stderr);
    fputc('c', stderr);
    fputc('\n', stderr);
    fputc(EOF, stderr);

    fflush(stderr);

    return 0;
}
