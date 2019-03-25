#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

typedef int (*mult)(int, int);

int main(int argc, char *argv[]) {
    // Open the module
    void * handle = dlopen("libfake.so", RTLD_NOW);
    if(handle == nullptr) {
        printf("Handle is null\n");
        return 1;
    }

    printf("Handle: %p\n", handle);

    // Extract the function handle

    mult f = (mult)dlsym(handle, "mult");
    printf("Mult func: %p\n", f);

    int result = f(25, 43);
    printf("Result: %i\n", result);

    return 0;
}