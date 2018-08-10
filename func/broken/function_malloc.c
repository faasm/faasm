#include <stdio.h>
#include <stdlib.h>

int main () {
    char *str;

    /* Initial memory allocation */
    str = (char *) malloc(15);
    strcpy(str, "faasm malloc");
    printf("String = %s,  Address = %u\n", str, str);

    /* Reallocating memory */
    str = (char *) realloc(str, 25);
    strcat(str, " added");
    printf("String = %s,  Address = %u\n", str, str);

    free(str);

    return(0);
}