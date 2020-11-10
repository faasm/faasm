#include "faasm/faasm.h"

#include <faasm/files.h>
#include <stdio.h>
#include <string.h>

/**
 * Test reading a file that we know should be faked
 */
int main(int argc, char* argv[])
{
    // Check we get a file length
    const char* path = "/etc/hosts";

    long length = faasm::getFileLength(path);
    if (length <= 0) {
        printf("Length of %s reported as %li\n", path, length);
        return 1;
    }

    FILE* fp = fopen(path, "r");

    char buff[2056];
    fgets(buff, 2056, fp);
    printf("Read from %s: \n%s\n", path, buff);

    const char* expected = "127.0.0.1\tlocalhost\n";

    // Check output is as expected
    if (strcmp(expected, buff) == 0) {
        printf("Content of %s as expected\n", path);
        return 0;
    } else {
        printf("Content of %s NOT as expected (%s)\n", path, buff);
        return 1;
    }
}
