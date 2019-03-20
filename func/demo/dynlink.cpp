#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


int main(int argc, char *argv[]) {
    const char* fileName = "lib/python3.7/site-packages/numpy/core/multiarray.so";

    void * handle = dlopen(fileName, RTLD_LAZY);

    printf("Handle: ");

    return 0;

}