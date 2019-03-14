#include <stdio.h>
#include <stdlib.h>

unsigned int emscripten_get_heap_size();

unsigned int emscripten_resize_heap(unsigned int size);

int main(int argc, char *argv[]) {
    printf("Hello from C\n");

    unsigned int heapSize = emscripten_get_heap_size();
    printf("Heap size: %u\n", heapSize);

    unsigned int ONE_MB = 1024 * 1024;
    unsigned int newSize = heapSize + (50 * ONE_MB);
    unsigned int heapResizeSuccess = emscripten_resize_heap(newSize);
    printf("Heap resize result: %u\n", heapResizeSuccess);

    heapSize = emscripten_get_heap_size();
    printf("Heap size: %u\n", heapSize);

    return 0;
}