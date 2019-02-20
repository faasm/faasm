#include "faasm/faasm.h"

#include <poll.h>

/** Attempts to poll multiple file descriptors */
namespace faasm {
    int exec(FaasmMemory *memory) {

        struct pollfd fdA;
        fdA.fd = 11;

        struct pollfd fdB;
        fdB.fd = 12;

        pollfd pollFds[2];
        pollFds[0] = fdA;
        pollFds[1] = fdB;
        nfds_t n = 2;

        poll(pollFds, n, 0);

        return 0;
    }
}
