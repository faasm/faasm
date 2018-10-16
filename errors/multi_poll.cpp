#include "faasm.h"

#include <poll.h>

/** Attempts to poll multiple file descriptors */
namespace faasm {
    int exec(FaasmMemory *memory) {

        struct pollfd fdA{
                .fd=1,
        };

        struct pollfd fdB{
                .fd=2,
        };

        pollfd pollFds[2];
        pollFds[0] = fdA;
        pollFds[1] = fdB;
        nfds_t n = 2;

        poll(pollFds, n, 0);

        return 0;
    }
}
