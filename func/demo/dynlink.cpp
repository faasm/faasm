#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


int main(int argc, char *argv[]) {
    const char* fileName = "libfake.wasm";

    // Open the module
    void * handle = dlopen(fileName, RTLD_LAZY);
    printf("Handle: %p\n", handle);

    // Extract the function handle
    typedef int (*doubleIntFunc)(int);
    auto f = (doubleIntFunc) dlsym(handle, "doubleInt");
    printf("Func: %p\n", f);

    // Print the result
    int result = f(100);
    printf("Result: %i\n", result);

    dlclose(handle);

    return 0;
}