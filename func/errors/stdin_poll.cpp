#include "faasm/faasm.h"

#include <poll.h>

/** Attempts to poll stdin */
FAASM_MAIN_FUNC() {
    struct pollfd stdinFd;
    stdinFd.fd = 0;

    pollfd pollFds[1];
    pollFds[0] = stdinFd;
    nfds_t n = 1;

    poll(pollFds, n, 0);

    return 0;
}
