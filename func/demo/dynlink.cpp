#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

typedef int (*multiply)(int, int);
typedef int (*multiplyGlobal)();

typedef int (*divide)(int, int);
typedef int (*divideGlobal)();

typedef int (*invokeSharedFunc)();

int main(int argc, char *argv[]) {
    // Open both the modules
    void * handleA = dlopen("libfakeLibA.so", RTLD_NOW);
    void * handleB = dlopen("libfakeLibB.so", RTLD_NOW);

    if(handleA == nullptr || handleB == nullptr) {
        printf("Importing dynamic libs failed\n");
        return 1;
    }

    printf("HandleA: %p\n", handleA);
    printf("HandleB: %p\n", handleB);

    // Extract functions
    auto mult = (multiply)dlsym(handleA, "multiply");
    auto multGlobal = (multiplyGlobal)dlsym(handleA, "multiplyGlobal");

    auto div = (divide)dlsym(handleB, "divide");
    auto divGlobal = (divideGlobal)dlsym(handleB, "divideGlobal");

    auto invokeShared = (invokeSharedFunc)dlsym(handleB, "invokeSharedFunc");

    // Call simple functions on each module
    int multResult = mult(4, 20);
    if(multResult != 80) {
        return 1;
    }

    int divideResult = div(100, 25);
    if(divideResult != 4) {
        return 1;
    }

    // Call functions on each module which depend on a shared global object
    // (Results depend on hard-coded variables in the modules)
    int multGlobalResult = multGlobal();
    if(multGlobalResult != 50) {
        return 1;
    }

    int divideGlobalResult = divGlobal();
    if(divideGlobalResult != 2) {
        return 1;
    }

    // Call a function on one module which requires a lookup in the GOT
    int sharedResult = invokeShared();
    if(sharedResult != 16) {
        return 1;
    }

    return 0;
}