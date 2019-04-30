#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char src[] = "alpha beta gamma delta";
    char dest[10];

    memcpy(dest, src, 8);
    dest[9] = '\0';

    if (strcmp(dest, "alpha be") != 0) {
        printf("memcpy not as expected (dest = %s)\n", dest);
        return 1;
    } else {
        printf("memcpy as expected \n");
        return 0;
    }
}