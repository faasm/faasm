#include "faasm/faasm.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>

/**
 * Test reading a file that we know should be faked
 */
namespace faasm {

    int exec(FaasmMemory *memory) {
        FILE *fp = fopen("/etc/hosts", "r");

//        printf("Size of FILE = %li\n", sizeof *fp);

        char buff[2056];
        fgets(buff, 2056, (FILE *) fp);
        printf("Read from /etc/hosts: \n%s\n", buff);

        const char* expected = "127.0.0.1\tlocalhost\n";

        // Check output is as expected
        if(strcmp(expected, buff) == 0) {
            printf("Content of /etc/hosts as expected\n");
            return 0;
        } else {
            printf("Content of /etc/hosts NOT as expected (%s)\n", buff);
            return 1;
        }
    }
}
