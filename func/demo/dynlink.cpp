#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <pwd.h>
#include <unistd.h>

// Note that the includes here are important for exporting functions required
// by the dynamically linked modules

typedef int (*multiply)(int, int);
typedef int (*multiplyGlobal)();

typedef int (*divide)(int, int);
typedef int (*divideGlobal)();

typedef int (*invokeSharedFunc)();

typedef int (*checkStack)();

int main(int argc, char *argv[]) {
    // Need mmap/ munmap in libA so must include here
    void* dummyPtr = mmap(NULL, 10 * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    munmap(dummyPtr, 10 * sizeof(int));

#if __WASM__ == 1
    // iprintf is an Emscripten-specific function which we need to include
    // to keep the dynamically linked libraries happy
    iprintf("mmaped location %p\n", dummyPtr);
#else
    printf("mmaped location %p\n", dummyPtr);
#endif

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
    auto checkS = (checkStack)dlsym(handleA, "checkStack");

    // Check that the stack and heap are as expected
    int stackCheckRes = checkS();
    if(stackCheckRes != 0) {
        printf("Stack and heap not arranged as expected");
        return 1;
    }

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