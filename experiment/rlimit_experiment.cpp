#include <sys/resource.h>
#include <cstdio>

void printRLimit(int resource, const char* name) {
    struct rlimit r{};
    getrlimit(resource, &r);

    rlim_t soft = r.rlim_cur;
    rlim_t hard = r.rlim_max;

    printf("%20s = %20li  %20li\n", name, soft, hard);
}

int main() {
    printf("Note, RLIM_INFINITY = %li\n\n", RLIM_INFINITY);

    printRLimit(RLIMIT_AS, "RLIMIT_AS");
    printRLimit(RLIMIT_CORE, "RLIMIT_CORE");
    printRLimit(RLIMIT_CPU, "RLIMIT_CPU");
    printRLimit(RLIMIT_DATA, "RLIMIT_DATA");
    printRLimit(RLIMIT_FSIZE, "RLIMIT_FSIZE");
    printRLimit(RLIMIT_LOCKS, "RLIMIT_LOCKS");
    printRLimit(RLIMIT_MEMLOCK, "RLIMIT_MEMLOCK");
    printRLimit(RLIMIT_MSGQUEUE, "RLIMIT_MSGQUEUE");
    printRLimit(RLIMIT_NICE, "RLIMIT_NICE");
    printRLimit(RLIMIT_NOFILE, "RLIMIT_NOFILE");
    printRLimit(RLIMIT_NPROC, "RLIMIT_NPROC");
    printRLimit(RLIMIT_RSS, "RLIMIT_RSS");
    printRLimit(RLIMIT_RTPRIO, "RLIMIT_RTPRIO");
    printRLimit(RLIMIT_RTTIME, "RLIMIT_RTTIME");
    printRLimit(RLIMIT_SIGPENDING, "RLIMIT_SIGPENDING");
    printRLimit(RLIMIT_STACK, "RLIMIT_STACK");
}