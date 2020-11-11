#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <vector>

int main(int argc, char* argv[])
{
    std::vector<unsigned int> sizes = { 8,  8,  32, 4,  240, 240, 56, 4,
                                        12, 12, 24, 24, 24,  512, 512 };

    // Run malloc in a loop
    void* allocs[sizes.size()];
    for (unsigned int i = 0; i < sizes.size(); i++) {
        allocs[i] = malloc(sizes.at(i));
    }

    // Free everything
    for (unsigned int i = 0; i < sizes.size(); i++) {
        free(allocs[i]);
    }

    // Run again and check that we reuse the same memory as before
    void* another = malloc(1024);
    if (another != allocs[0]) {
        printf("Not reusing malloced memory\n");
        return 1;
    }

    auto strPtr = (char*)another;
    strcpy(strPtr, "this is malloc");
    printf("Malloced string: %s\n", strPtr);

    free(another);

    // Malloc something big
    void* bigPtr = malloc(1024 * 1024);
    if (bigPtr == nullptr) {
        printf("Big malloc failed\n");
        return 1;
    }

    auto bigStrPtr = (char*)bigPtr;
    strcpy(bigStrPtr, "this is malloc big");
    printf("Malloced big: %s\n", bigStrPtr);

    free(bigPtr);

    return 0;
}
