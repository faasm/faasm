#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


int main(int argc, char *argv[]) {
    const char* fileName = "libfaasm.wasm";

    // Open the module
    void * handle = dlopen(fileName, RTLD_LAZY);
    printf("Handle: %p\n", handle);

    // Extract the function handle
    int (*foo)(int);
    *(void **) (&foo) = dlsym(handle, "foo");

    // Print the result
    int result = (*foo)(2);
    printf("Result: %i\n", result);

    return 0;

}