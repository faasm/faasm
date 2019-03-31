#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    void *ptr = malloc(2048);

    if (ptr == nullptr) {
        return 1;
    } else {
        return 0;
    }
}