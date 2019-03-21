#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


int main(int argc, char *argv[]) {
    const char* fileName = "libfake.wasm";

    // Open the module
    void * handle = dlopen(fileName, RTLD_LAZY);
    printf("Handle: %p\n", handle);

    // Extract the function handle
    int (*doubleInt)(int);
    *(void **) (&doubleInt) = dlsym(handle, "_doubleInt");

    // Print the result
    int result = (*doubleInt)(100);
    printf("Result: %i\n", result);

    return 0;
}