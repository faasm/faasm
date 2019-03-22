#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


int main(int argc, char *argv[]) {
    // Open the module
    void * handle = dlopen("libfake.wasm", RTLD_LAZY);
    if(handle == nullptr) {
        printf("Handle is null\n");
        return 1;
    }

    printf("Handle: %p\n", handle);

    // Extract the function handle
    typedef int (*doubleIntFunc)(int);
    void * rawPtr = dlsym(handle, "doubleInt");
    printf("Raw pointer: %p\n", rawPtr);

    auto f = (doubleIntFunc) rawPtr;
    printf("Func: %p\n", f);

    // Print the result
    int result = f(100);
    printf("Result: %i\n", result);

    dlclose(handle);

    return 0;
}