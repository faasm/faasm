#include <stdio.h>

int main() {
    printf("Opening file");

    int num;
    FILE *fptr;
    fptr = fopen("/tmp/foobar.txt", "w");

    printf("File opened");

//   if(fptr == NULL)
//   {
//      printf("Error!");
//   }

    fprintf(fptr, "Hello world!");

    printf("File written");

    fclose(fptr);

    printf("File closed");
}
