#include "faasm/faasm.h"

#include <fcntl.h>
#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        int fd = open("/etc/hosts", O_RDONLY);

        if(fd <= 0) {
            printf("Unable to open valid file\n");
            return 1;
        }
        
        // Duplicate the fd
        int newFd = fcntl(fd, F_DUPFD, 0);

        if(newFd <= 0) {
            printf("Unable to duplicate fd with fcntl\n");
            return 1;
        }

        return 0;
    }
}
