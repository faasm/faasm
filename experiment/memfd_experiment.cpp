#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <cstring>

int main() {
    printf("Hi\n");

    std::string outStr = "hi there!\n";
    int f = memfd_create("foobar", 0);
    ftruncate(f, outStr.size());

    ssize_t written = write(f, outStr.c_str(), outStr.size());
    if (written == -1) {
        printf("Write error %s\n", strerror(errno));
    }
    printf("Written %li\n", written);

    char *actual = new char[outStr.size()];
    lseek(f, 0, SEEK_SET);
    ssize_t readRes = read(f, actual, outStr.size());
    if(readRes == -1) {
        printf("Read error %s\n", strerror(errno));
    }
    printf("Read %li\n", readRes);

    printf("Actual %s\n", actual);
}