#include <stdio.h>

int main() {
   printf("FOOBAR");

   int num;
   FILE *fptr;
   fptr = fopen("/tmp/foobar.txt","w");

   if(fptr == NULL)
   {
      printf("Error!");   
   }

   fprintf(fptr,"Hello world!");
   fclose(fptr);
}
