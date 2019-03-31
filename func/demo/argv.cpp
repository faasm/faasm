#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("argc = %i, argv[0] = %s\n", argc, argv[0]);

    return strcmp(argv[0], "function.wasm");
}