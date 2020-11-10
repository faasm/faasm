#include "faasm/faasm.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char buf[1024];

    char* res = getcwd(buf, 1024);
    if (res != buf) {
        printf("Expected to get buf back (%p), but got (%p)\n", buf, res);
        return 1;
    }

    const char* expected = "/bin";
    if (strcmp(res, expected) != 0) {
        printf("Unexpected working dir (expected %s, got %s)\n", expected, res);
        return 1;
    } else {
        printf("Got expected working dir %s\n", res);
    }

    return 0;
}
